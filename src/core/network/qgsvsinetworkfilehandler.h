/***************************************************************************
                         qgsvsinetworkfilehandler.h
                         --------------------
    begin                : May 2025
    copyright            : (C) 2025 by Jan Dalheimer
    email                : jan dot dalheimer at sweco dot se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVSINETWORKFILEHANDLER_H
#define QGSVSINETWORKFILEHANDLER_H

#include <cpl_vsi_virtual.h>
#include <gdal_version.h>

#include "qgsblockingnetworkrequest.h"

#include <QNetworkReply>
#include <QReadWriteLock>
#include <QUrlQuery>

class QgsVSINetworkHandle;
//class QgsVSINetworkStreamingHandle;

struct FileProp {
    enum class Exists { UNKNOWN, NO, YES } exists = Exists::UNKNOWN;
    int statusCode = 0;
    vsi_l_offset fileSize = 0;
    time_t mTime = 0;
    QUrl redirectURL;
    bool hasFileSize = false;
    int mode = 0;
    QList<QPair<QByteArray, QByteArray>> headers;
};

class QgsVSINetworkFileHandler : public VSIFilesystemHandler
{
    CPL_DISALLOW_COPY_ASSIGN(QgsVSINetworkFileHandler)

    mutable QReadWriteLock mCachedPropertiesLock;
    QHash<QUrl, FileProp> mCachedProperties;

  protected:
    virtual QgsVSINetworkHandle *CreateFileHandle(const char *pszFilename);

  public:
    QgsVSINetworkFileHandler() = default;

    VSIVirtualHandle *Open(const char *pszFilename, const char *pszAccess, bool bSetError, CSLConstList papszOptions) override;
    int Stat(const char *pszFilename, VSIStatBufL *pStatBuf, int nFlags) override;
    const char *GetActualURL(const char *pszFilename) override;
    char **GetFileMetadata(const char *pszFilename, const char *pszDomain, CSLConstList papszOptions) override;
    bool SupportsSequentialWrite(const char *, bool) override { return false; }
    bool SupportsRandomWrite(const char *, bool) override { return false; }
    int HasOptimizedReadMultiRange(const char *) override { return false; }
    bool IsLocal(const char *) override { return false; }

    std::string GetFSPrefix() const { return "/vsiqgis"; }
    /*std::string GetStreamingFilename(const std::string &osFilename) const override
    {
      if (STARTS_WITH(osFilename.c_str(), GetFSPrefix().c_str()))
        return "/vsiqgis_streaming" + osFilename.substr(GetFSPrefix().size());
      return osFilename;
    }*/

    FileProp cachedProperties(const QUrl &url) const
    {
      QReadLocker locker(&mCachedPropertiesLock);
      return mCachedProperties.value(url);
    }
    void setCachedProperties(const QUrl &url, const FileProp &properties)
    {
      QWriteLocker locker(&mCachedPropertiesLock);
      mCachedProperties.insert(url, properties);
    }
};

class QgsVSINetworkHandle : public VSIVirtualHandle
{
    vsi_l_offset curOffset = 0;

    QgsBlockingNetworkRequest::ErrorCode mLastError;
    bool mEOF = false;

    FileProp mProperties;

    vsi_l_offset GetFileSize(bool bSetError)
    {
      return GetFileSizeOrHeaders(bSetError, false);
    }

    QgsVSINetworkFileHandler *mHandler;
    const QHash<QString, QString> mOptions;
    const QUrl mUrl;

    static QHash<QString, QString> parseOptions(QgsVSINetworkFileHandler *handler, const char *filename)
    {
      const QString fileName = QString::fromUtf8(filename);
      if ( !fileName.startsWith(QString::fromStdString(handler->GetFSPrefix()) )) {
        return {};
      }
      const QString withoutPrefix = fileName.mid( handler->GetFSPrefix().size() );
      if (withoutPrefix.startsWith(QLatin1Char('/'))) {
        return {};
      } else if (withoutPrefix.startsWith(QLatin1Char('?'))) {
        const auto query = QUrlQuery(withoutPrefix.mid(1));
        QHash<QString, QString> result;
        for ( auto [k, v] : query.queryItems() ) {
          result.insert( k, v );
        }
        return result;
      } else {
        return {};
      }
    }
    static QUrl parseUrl(QgsVSINetworkFileHandler *handler, const char *filename)
    {
      const QString fileName = QString::fromUtf8(filename);
      if ( !fileName.startsWith(QString::fromStdString(handler->GetFSPrefix()) )) {
        return QUrl();
      }
      const QString withoutPrefix = fileName.mid( handler->GetFSPrefix().size() );
      if (withoutPrefix.startsWith(QLatin1Char('/'))) {
        return QUrl(withoutPrefix.mid(1));
      } else if (withoutPrefix.startsWith(QLatin1Char('?'))) {
        const auto query = QUrlQuery(withoutPrefix.mid(1));
        return QUrl(query.queryItemValue(QStringLiteral("url"), QUrl::FullyDecoded));
      } else {
        return QUrl();
      }
    }

  public:
    QgsVSINetworkHandle(QgsVSINetworkFileHandler *handler, const char *filename)
      : mHandler(handler), mOptions(parseOptions(handler, filename)), mUrl(parseUrl(handler, filename))
    {
      mProperties = mHandler->cachedProperties(mUrl);
    }
    virtual ~QgsVSINetworkHandle();

    QUrl url() const { return mUrl; }
    const char *GetURL() const
    {
      return mUrl.toString().toUtf8().constData();
    }

    vsi_l_offset GetFileSizeOrHeaders(bool bSetError, bool bGetHeaders);

    int Seek(vsi_l_offset nOffset, int nWhence) override
    {

      if (nWhence == SEEK_SET)
      {
        curOffset = nOffset;
      }
      else if (nWhence == SEEK_CUR)
      {
        curOffset = curOffset + nOffset;
      }
      else
      {
        curOffset = GetFileSize(false) + nOffset;
      }
      mEOF = false;
      return 0;
    }
    vsi_l_offset Tell() override { return curOffset; }
    size_t Read(void *pBuffer, size_t nSize, size_t nCount) override
    {
      QgsBlockingNetworkRequest request;
      if ( mOptions.contains("authcfg") ) {
        request.setAuthCfg( mOptions.value("authcfg"));
      }
      QNetworkRequest req(mUrl);
      req.setRawHeader(QByteArrayLiteral("Range"), QStringLiteral("bytes=%1-%2").arg(curOffset).arg(curOffset + nSize*nCount-1).toLatin1());

      QgsFeedback *feedback = new QgsFeedback;
      mActiveRequests.insert( feedback );
      mLastError = request.get(req, false, feedback);
      mActiveRequests.remove( feedback );
      delete feedback;

      QgsNetworkReplyContent reply = request.reply();
      const int statusCode = reply.attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
      if (reply.error() != QNetworkReply::NoError || statusCode != 206) {
        return 0;
      }

      const size_t ret = std::min(reply.content().size() / nSize, nCount);
      if (ret != nCount)
        mEOF = true;

      if (ret > 0) {
        memcpy(pBuffer, reply.content().constData(), std::min((size_t)reply.content().size(), nSize * nCount));
      }

      curOffset = curOffset + reply.content().size();
      return ret;
    }
    int ReadMultiRange(int nRanges, void **ppData, const vsi_l_offset *panOffsets, const size_t *panSizes) override
    {
      return VSIVirtualHandle::ReadMultiRange( nRanges, ppData, panOffsets, panSizes );
    }

    size_t Write(const void *, size_t, size_t) override
    {
      return 0;
    }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3, 10, 0)
    void ClearErr() override
    {
      // TODO
    }
    int Error() override
    {
      return mLastError == QgsBlockingNetworkRequest::NoError ? FALSE : TRUE;
    }
    void Interrupt() override
    {
      for ( QgsFeedback *request : qAsConst( mActiveRequests ) ) {
        request->cancel();
      }
    }
#endif

    int Eof() override { return mEOF ? TRUE : FALSE; }
    int Flush() override { return 0; }
    int Close() override { return 0; }

    bool HasPRead() const override { return false; }

    size_t PRead(void *, size_t,
                  vsi_l_offset) const override {
      return -1;
    }

    void AdviseRead(int, const vsi_l_offset *,
                     const size_t *) override {}

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3, 9, 0)
    size_t GetAdviseReadTotalBytesLimit() const override
    {
      // 100 MB, the same as the default value for CPL_VSIL_CURL_ADVISE_READ_TOTAL_BYTES_LIMIT
      return 104857600;
    }
#endif

  private:
    QSet< QgsFeedback* > mActiveRequests;
};

/*class QgsVSINetworkFileHandlerStreaming : public QgsVSINetworkFileHandlerBase
{
    CPL_DISALLOW_COPY_ASSIGN(QgsVSINetworkFileHandlerStreaming)

  public:
    QgsVSINetworkFileHandlerStreaming() = default;

    std::string GetFSPrefix() const override { return "/vsiqgis_streaming/"; }

#if GDAL_COMPUTE_VERSION(3, 11, 0) <= GDAL_VERSION_NUM
    std::string GetNonStreamingFilename(const std::string &osFilename) const override
    {
      if (STARTS_WITH(osFilename.c_str(), GetFSPrefix().c_str()))
        return "/vsiqgis/" + osFilename.substr(GetFSPrefix().size());
      return osFilename;
    }
#endif

    const char *GetOptions() override
    {
      return VSIGetFileSystemOptions("/vsiqgis/");
    }
};*/

#endif // QGSVSINETWORKFILEHANDLER_H
