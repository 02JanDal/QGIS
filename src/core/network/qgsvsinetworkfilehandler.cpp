/***************************************************************************
                         qgsvsinetworkfilehandler.cpp
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

#include "qgsvsinetworkfilehandler.h"

QgsVSINetworkHandle *QgsVSINetworkFileHandler::CreateFileHandle(const char *pszFilename)
{
  QgsVSINetworkHandle *handle = new QgsVSINetworkHandle(this, pszFilename);
  if ( !handle->url().isValid() ) {
    delete handle;
    return nullptr;
  }
  return handle;
}

VSIVirtualHandle *QgsVSINetworkFileHandler::Open(const char *pszFilename, const char *pszAccess, bool bSetError, CSLConstList)
{
  if (strchr(pszAccess, 'w') != nullptr || strchr(pszAccess, '+') != nullptr)
  {
    if (bSetError)
    {
      VSIError(VSIE_FileError, "Only read-only mode is supported for /vsiqgis");
    }
    return nullptr;
  }

  return CreateFileHandle(pszFilename);
}

int QgsVSINetworkFileHandler::Stat(const char *pszFilename, VSIStatBufL *pStatBuf, int nFlags)
{

  std::unique_ptr<QgsVSINetworkHandle> poHandle(CreateFileHandle(pszFilename));
  if (poHandle == nullptr)
    return -1;

  memset(pStatBuf, 0, sizeof(VSIStatBufL));

  if ((nFlags & VSI_STAT_CACHE_ONLY) == 0)
  {
    poHandle->GetFileSizeOrHeaders(false, true);
  }
  const FileProp properties = cachedProperties(poHandle->url());
  pStatBuf->st_size = properties.fileSize;
  pStatBuf->st_mtim.tv_sec = properties.mTime;
  pStatBuf->st_mode = properties.mode;

  return 0;
}

const char *QgsVSINetworkFileHandler::GetActualURL(const char *pszFilename)
{
  if (!STARTS_WITH_CI(pszFilename, GetFSPrefix().c_str()))
    return pszFilename;
  auto poHandle = std::unique_ptr<QgsVSINetworkHandle>(
    CreateFileHandle(pszFilename));
  if (poHandle == nullptr)
    return pszFilename;
  return CPLSPrintf("%s", poHandle->GetURL());
}

char **QgsVSINetworkFileHandler::GetFileMetadata(const char *pszFilename, const char *pszDomain, CSLConstList)
{
  if (pszDomain == nullptr || !EQUAL(pszDomain, "HEADERS"))
    return nullptr;
  std::unique_ptr<QgsVSINetworkHandle> poHandle(CreateFileHandle(pszFilename));
  if (poHandle == nullptr)
    return nullptr;

  poHandle->GetFileSizeOrHeaders(true, true);
  const auto headers = cachedProperties(poHandle->url()).headers;

  char **result = new char*[headers.size()+1];
  for (int i = 0; i < headers.size(); ++i) {
    QByteArray combined = headers.at(i).first + QByteArrayLiteral(": ") + headers.at(i).second;
    result[i] = combined.data();
  }
  result[headers.size()] = nullptr;

  return CSLDuplicate(result);
}

QgsVSINetworkHandle::~QgsVSINetworkHandle() {}

vsi_l_offset QgsVSINetworkHandle::GetFileSizeOrHeaders(bool setError, bool getHeaders)
{
  if (mProperties.hasFileSize && !getHeaders) {
    return mProperties.fileSize;
  }

  mProperties.hasFileSize = true;

  QgsBlockingNetworkRequest request;
  if ( mOptions.contains("authcfg") ) {
    request.setAuthCfg( mOptions.value("authcfg"));
  }
  QNetworkRequest req(mUrl);
  const QgsBlockingNetworkRequest::ErrorCode error = request.head(req, true);
  if (setError && error != QgsBlockingNetworkRequest::NoError) {
    mLastError = error;
  }

  mProperties.statusCode = request.reply().attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (mProperties.statusCode >= 200 && mProperties.statusCode < 300) {
    mProperties.exists = FileProp::Exists::YES;
    mProperties.mode = S_IFREG;

    mProperties.headers = request.reply().rawHeaderPairs();

    const QByteArray lastModified = request.reply().rawHeader(QByteArrayLiteral("Last-Modified"));
    if (!lastModified.isNull()) {
      mProperties.mTime = QDateTime::fromString(QString::fromLatin1(lastModified), QStringLiteral("ddd, dd MM yyyy HH:mm:ss t")).toSecsSinceEpoch();
    }

    const QByteArray contentLength = request.reply().rawHeader(QByteArrayLiteral("Content-Length"));
    if (!contentLength.isNull()) {
      bool ok = false;
      mProperties.fileSize = contentLength.toLongLong(&ok);
      mProperties.hasFileSize = ok;
      if (!ok) {
        return -1;
      }
    }

    mHandler->setCachedProperties(mUrl, mProperties);

    return mProperties.fileSize;
  } else if (mProperties.statusCode == 404) {
    mProperties.exists = FileProp::Exists::NO;
    return -1;
  } else {
    mProperties.exists = FileProp::Exists::UNKNOWN;
    return -1;
  }
}
