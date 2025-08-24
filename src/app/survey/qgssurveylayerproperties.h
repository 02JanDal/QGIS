/***************************************************************************
  qgstiledscenelayerproperties.h
  --------------------------------------
  Date                 : June 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSURVEYLAYERPROPERTIES_H
#define QGSSURVEYLAYERPROPERTIES_H

#include "qgscoordinatereferencesystem.h"
#include "qgslayerpropertiesdialog.h"
#include "ui_qgssurveylayerpropertiesbase.h"
#include "qgis_app.h"

class QgsSurveyLayer;
class QgsMessageBar;
class QgsMetadataWidget;

class APP_EXPORT QgsSurveyLayerProperties : public QgsLayerPropertiesDialog, private Ui::QgsSurveyLayerPropertiesBase
{
    Q_OBJECT
  public:
    QgsSurveyLayerProperties( QgsSurveyLayer *layer, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent = nullptr, Qt::WindowFlags = QgsGuiUtils::ModalDialogFlags );

  protected slots:
    void syncToLayer() final;
    void apply() final;
    void rollback() final;

  private slots:
    void aboutToShowStyleMenu();
    void showHelp();
    void crsChanged( const QgsCoordinateReferenceSystem &crs );

  private:
    QgsSurveyLayer *mLayer = nullptr;

    QAction *mActionLoadMetadata = nullptr;
    QAction *mActionSaveMetadataAs = nullptr;

    QgsMetadataWidget *mMetadataWidget = nullptr;

    QgsCoordinateReferenceSystem mBackupCrs;
};

#endif // QGSSURVEYLAYERPROPERTIES_H
