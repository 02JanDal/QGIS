/***************************************************************************
   qgsassetguiutils.cpp

 ---------------------
 begin                : 2025-06-11
 copyright            : (C) Jan Dalheimer
 email                : jan at dalheimer dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsassetguiutils.h"

#include "qgsfeatureasset.h"
#include "qgsmessagebar.h"
#include "qgsnetworkcontentfetchertask.h"

#include <QFileInfo>
#include <qgsapplication.h>

QgsTask *QgsAssetGuiUtils::downloadAsset(const QgsFeatureAsset &asset, const QString &destinationFolder, const QString &authCfg, QgsMessageBar *messageBar , QString *destinationFile)
{
  QgsNetworkContentFetcherTask *fetcher = new QgsNetworkContentFetcherTask( asset.href(), authCfg, QgsTask::CanCancel, tr( "Downloading STAC asset" ) );

  QObject::connect( fetcher, &QgsNetworkContentFetcherTask::errorOccurred, fetcher, [messageBar]( QNetworkReply::NetworkError, const QString &errorMsg ) {
    if ( messageBar )
      messageBar->pushMessage(
        tr( "Error downloading asset" ),
        errorMsg,
        Qgis::MessageLevel::Critical
        );
  } );

  QObject::connect( fetcher, &QgsNetworkContentFetcherTask::fetched, fetcher, [fetcher, destinationFolder, messageBar, destinationFile] {
    QNetworkReply *reply = fetcher->reply();
    if ( !reply || reply->error() != QNetworkReply::NoError )
    {
      // canceled or failed
      return;
    }
    else
    {
      const QString fileName = fetcher->contentDispositionFilename().isEmpty() ? reply->url().fileName() : fetcher->contentDispositionFilename();
      QFileInfo fi( fileName );
      QFile file( QStringLiteral( "%1/%2" ).arg( destinationFolder, fileName ) );
      int i = 1;
      while ( file.exists() )
      {
        QString uniqueName = QStringLiteral( "%1/%2(%3)" ).arg( destinationFolder, fi.baseName() ).arg( i++ );
        if ( !fi.completeSuffix().isEmpty() )
          uniqueName.append( QStringLiteral( ".%1" ).arg( fi.completeSuffix() ) );
        file.setFileName( uniqueName );
      }

      bool failed = false;
      if ( file.open( QIODevice::WriteOnly ) )
      {
        const QByteArray data = reply->readAll();
        if ( file.write( data ) < 0 )
          failed = true;

        file.close();
      }
      else
      {
        failed = true;
      }

      if ( failed )
      {
        if ( messageBar )
          messageBar->pushMessage(
            tr( "Error downloading STAC asset" ),
            tr( "Could not write to file %1" ).arg( file.fileName() ),
            Qgis::MessageLevel::Critical
            );
      }
      else
      {
        if (destinationFile) {
          *destinationFile = file.fileName();
        }

        if ( messageBar )
          messageBar->pushMessage(
            tr( "STAC asset downloaded" ),
            file.fileName(),
            Qgis::MessageLevel::Success
            );
      }
    }
  } );

  QgsApplication::taskManager()->addTask( fetcher );

  return fetcher;
}
