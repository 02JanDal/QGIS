/***************************************************************************
    qgsfeatureasset.h
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

#ifndef QGSFEATUREASSET_H
#define QGSFEATUREASSET_H


#include "qgis_core.h"
#include "qgsmimedatautils.h"

#include <QString>
#include <QStringList>
#include <optional>

/**
 * \ingroup core
 * \brief Class for storing a metadata about an asset for a feature.
 *
 * \since QGIS 3.xx
 */
class CORE_EXPORT QgsFeatureAsset
{
  public:
    // Default constructor
    QgsFeatureAsset() = default;

    //! Constructor
    QgsFeatureAsset( const QString &id, const QString &href,
                  const QString &title,
                  const QString &description,
                  const QString &mediaType,
                     const QStringList &roles = QStringList(),
                     const QVariantMap &metadata = QVariantMap() );

    QString id() const;

    //! Returns the URI to the asset object
    QString href() const;

    //! Returns the the displayed title for clients and users.
    QString title() const;

    /**
     * Returns a description of the Asset providing additional details, such as how it was processed or created.
     * CommonMark 0.29 syntax MAY be used for rich text representation.
     */
    QString description() const;

    //! Returns the media type of the asset
    QString mediaType() const;

    /**
     * Returns the roles assigned to the asset.
     * Roles are used to describe the purpose of the asset (eg. thumbnail, data etc).
     */
    QStringList roles() const;

    QVariantMap metadata() const;

    /**
     * Returns whether the asset is in a cloud optimized format like COG or COPC
     * \since QGIS 3.42
     */
    bool isCloudOptimized() const;

    /**
     * Returns the format name for cloud optimized formats
     * \since QGIS 3.42
     */
    QString formatName() const;

    /**
     * Returns a uri for the asset if it is a cloud optimized file like COG or COPC
     * \since QGIS 3.42
     */
    QgsMimeDataUtils::Uri uri( const QString &authCfg = QString() ) const;
    QgsMimeDataUtils::Uri uriFromLayer( QgsVectorLayer *layer = nullptr ) const;

    Qgis::LayerType layerType() const;

    friend QDataStream &operator<<( QDataStream &stream, const QgsFeatureAsset &asset )
    {
      stream << asset.mId << asset.mHref << asset.mTitle << asset.mDescription << asset.mMediaType << asset.mRoles << asset.mMetadata;
      return stream;
    }
    friend QDataStream &operator>>( QDataStream &stream, QgsFeatureAsset &asset )
    {
      QVariant roles;
      QVariant metadata;
      stream >> asset.mId >> asset.mHref >> asset.mTitle >> asset.mDescription >> asset.mMediaType >> roles >> metadata;
      asset.mRoles = roles.toStringList();
      asset.mMetadata = metadata.toMap();
      return stream;
    }

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &obj);
    QVariantMap toMap() const;

  private:
    QString mId;
    QString mHref;
    QString mTitle;
    QString mDescription;
    QString mMediaType;
    QStringList mRoles;
    QVariantMap mMetadata;
};

Q_DECLARE_METATYPE( QgsFeatureAsset )

#endif // QGSFEATUREASSET_H
