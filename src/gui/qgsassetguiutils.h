/***************************************************************************
   qgsassetguiutils.h

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

#ifndef QGSASSETGUIUTILS_H
#define QGSASSETGUIUTILS_H

#include <QCoreApplication>
#include "qgis_gui.h"

class QgsTask;
class QgsMessageBar;
class QgsFeatureAsset;

class GUI_EXPORT QgsAssetGuiUtils
{
    Q_DECLARE_TR_FUNCTIONS( QgsAssetGuiUtils )
  public:
    static QgsTask *downloadAsset(const QgsFeatureAsset &asset , const QString &destinationFolder, const QString &authCfg, QgsMessageBar *messageBar, QString *destinationFile = nullptr );
};

#endif // QGSASSETGUIUTILS_H
