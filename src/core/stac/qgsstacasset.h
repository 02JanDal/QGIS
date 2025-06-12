/***************************************************************************
    qgsstacasset.h
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

#ifndef QGSSTACASSET_H
#define QGSSTACASSET_H

#include "qgis_core.h"
#include "qgsfeatureasset.h"

/**
 * \ingroup core
 * \brief Class for storing a STAC asset's data.
 *
 * \since QGIS 3.44
 */
class CORE_EXPORT QgsStacAsset : public QgsFeatureAsset
{
  public:
    QgsStacAsset() = default;

    QgsStacAsset( const QString &id, const QString &href,
                  const QString &title,
                  const QString &description,
                  const QString &mediaType,
                  const QStringList &roles = QStringList(),
                  const QVariantMap &metadata = QVariantMap() );
};

Q_DECLARE_METATYPE( QgsStacAsset )

#endif // QGSSTACASSET_H
