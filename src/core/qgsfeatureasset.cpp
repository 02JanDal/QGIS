/***************************************************************************
    qgsfeatureasset.cpp
    ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatasourceuri.h"
#include "qgsfeatureasset.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QMimeDatabase>
#include <QUrl>
#include <QUrlQuery>
#include <qgsvectorlayer.h>

QgsFeatureAsset::QgsFeatureAsset(const QString &id, const QString &href,
                                  const QString &title,
                                  const QString &description,
                                  const QString &mediaType,
                                  const QStringList &roles , const QVariantMap &metadata)
  : mId( id )
  , mHref( href )
  , mTitle( title )
  , mDescription( description )
  , mMediaType( mediaType )
  , mRoles( roles )
  , mMetadata( metadata )
{
}

QString QgsFeatureAsset::id() const
{
  return mId;
}

QString QgsFeatureAsset::href() const
{
  return mHref;
}

QString QgsFeatureAsset::title() const
{
  return mTitle;
}

QString QgsFeatureAsset::description() const
{
  return mDescription;
}

QString QgsFeatureAsset::mediaType() const
{
  return mMediaType;
}

QStringList QgsFeatureAsset::roles() const
{
  return mRoles;
}

QVariantMap QgsFeatureAsset::metadata() const
{
  return mMetadata;
}

bool QgsFeatureAsset::isCloudOptimized() const
{
  const QString format = formatName();
  return format == QLatin1String( "COG" ) ||
         format == QLatin1String( "COPC" ) ||
         format == QLatin1String( "EPT" );
}

QString QgsFeatureAsset::formatName() const
{
  if ( mMediaType == QLatin1String( "image/tiff; application=geotiff; profile=cloud-optimized" ) ||
       mMediaType == QLatin1String( "image/vnd.stac.geotiff; cloud-optimized=true" ) )
    return QStringLiteral( "COG" );
  else if ( mMediaType == QLatin1String( "application/vnd.laszip+copc" ) )
    return QStringLiteral( "COPC" );
  else if ( mHref.endsWith( QLatin1String( "/ept.json" ) ) )
    return QStringLiteral( "EPT" );
  else if ( mMediaType == QLatin1String( "application/geo+json" ) )
    return QStringLiteral( "GeoJSON" );
  return QMimeDatabase().mimeTypeForName( mMediaType ).comment();
}

QgsMimeDataUtils::Uri QgsFeatureAsset::uri( const QString &authCfg ) const
{
  QgsMimeDataUtils::Uri uri;
  QUrl url( href() );
  if ( formatName() == QLatin1String( "COG" ) )
  {
    uri.layerType = QStringLiteral( "raster" );
    uri.providerKey = QStringLiteral( "gdal" );
    if ( href().startsWith( QLatin1String( "http" ), Qt::CaseInsensitive ) ||
         href().startsWith( QLatin1String( "ftp" ), Qt::CaseInsensitive ) )
    {
      QUrlQuery query;
      query.addQueryItem( QStringLiteral( "url" ), href() );
      if ( !authCfg.isEmpty() )
      {
        query.addQueryItem( QStringLiteral( "authcfg" ), authCfg );
      }
      uri.uri = QStringLiteral( "/vsiqgis?%1" ).arg( query.toString( QUrl::FullyEncoded ) );
    }
    else if ( href().startsWith( QLatin1String( "s3://" ), Qt::CaseInsensitive ) )
    {
      uri.uri = QStringLiteral( "/vsis3/%1" ).arg( href().mid( 5 ) );
    }
    else
    {
      uri.uri = href();
    }
  }
  else if ( formatName() == QLatin1String( "COPC" ) )
  {
#if VERSION_INT < 34200
    uri.layerType = QStringLiteral( "pointcloud" );
#else
    uri.layerType = QStringLiteral( "point-cloud" );
#endif
    uri.providerKey = QStringLiteral( "copc" );
    uri.uri = href();
  }
  else if ( formatName() == QLatin1String( "EPT" ) )
  {
#if VERSION_INT < 34200
    uri.layerType = QStringLiteral( "pointcloud" );
#else
    uri.layerType = QStringLiteral( "point-cloud" );
#endif
    uri.providerKey = QStringLiteral( "ept" );
    uri.uri = href();
  }
  else if ( formatName() == QLatin1String( "GeoJSON" ) )
  {
    uri.layerType = QStringLiteral( "vector" );
    uri.providerKey = QStringLiteral( "ogr" );
    QUrlQuery query;
    query.addQueryItem( QStringLiteral( "url" ), href() );
    if ( !authCfg.isEmpty() )
    {
      query.addQueryItem( QStringLiteral( "authcfg" ), authCfg );
    }
    uri.uri = QStringLiteral( "/vsiqgis?%1" ).arg( query.toString( QUrl::FullyEncoded ) );
  }
  else
  {
    return {};
  }

  uri.name = title().isEmpty() ? url.fileName() : title();

  return uri;
}

QgsMimeDataUtils::Uri QgsFeatureAsset::uriFromLayer(QgsVectorLayer *layer) const
{
  return uri( QgsDataSourceUri( layer->dataProvider()->dataSourceUri() ).authConfigId() );
}

Qgis::LayerType QgsFeatureAsset::layerType() const
{
  if ( formatName() == QLatin1String( "COG" ) )
  {
    return Qgis::LayerType::Raster;
  }
  else if ( formatName() == QLatin1String( "COPC" ) || formatName() == QLatin1String( "EPT" ) )
  {
    return Qgis::LayerType::PointCloud;
  }
  else if ( formatName() == QLatin1String( "GeoJSON" ) )
  {
    return Qgis::LayerType::Vector;
  }
  else {
    // TODO: make this function return an optional instead
    throw std::runtime_error("Unknown layer type");
  }
}

QJsonObject QgsFeatureAsset::toJson() const
{
  return QJsonObject({
                        {QStringLiteral("id"), mId},
                        {QStringLiteral("href"), mHref},
                        {QStringLiteral("title"), mTitle},
                        {QStringLiteral("description"), mDescription},
                        {QStringLiteral("mediaType"), mMediaType},
                        {QStringLiteral("roles"), QJsonArray::fromStringList( mRoles )},
                        {QStringLiteral("metadata"), QJsonObject::fromVariantMap( mMetadata )}
  });
}

bool QgsFeatureAsset::fromJson(const QJsonObject &obj)
{
  mId = obj.value( QStringLiteral("id") ).toString();
  mHref = obj.value( QStringLiteral("href") ).toString();
  mTitle = obj.value( QStringLiteral("title") ).toString();
  mDescription = obj.value( QStringLiteral("description") ).toString();
  mMediaType = obj.value( QStringLiteral("mediaType") ).toString();
  mMetadata = obj.value( QStringLiteral("metadata") ).toObject().toVariantMap();

  const QJsonArray roles = obj.value( QStringLiteral("roles") ).toArray();
  mRoles.clear();
  for ( const QJsonValue &value : roles )
  {
    mRoles.append( value.toString() );
  }

  return true;
}

QVariantMap QgsFeatureAsset::toMap() const
{
  return QVariantMap({
    {QStringLiteral("id"), mId},
    {QStringLiteral("href"), mHref},
    {QStringLiteral("title"), mTitle},
    {QStringLiteral("description"), mDescription},
    {QStringLiteral("mediaType"), mMediaType},
    {QStringLiteral("roles"), QJsonArray::fromStringList( mRoles )},
                        {QStringLiteral("metadata"), mMetadata}
  });
}
