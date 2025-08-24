/***************************************************************************
    qgssurveyrendererpropertieswidget.h
    ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSURVEYRENDERERPROPERTIESWIDGET_H
#define QGSSURVEYRENDERERPROPERTIESWIDGET_H

#include "qgis_sip.h"
#include "qgis_gui.h"

#include "ui_qgssurveyrendererpropertiesdialogbase.h"
#include "qgsmaplayerconfigwidget.h"

class QgsSurveyLayer;
class QgsStyle;
class QgsSurveyRendererWidget;
class QgsMapCanvas;
class QgsSymbolWidgetContext;
class QgsMessageBar;

/**
 * \ingroup gui
 * \brief A generic widget for setting the 2D renderer for a survey layer.
 *
 * \since QGIS 3.xx
 */
class GUI_EXPORT QgsSurveyRendererPropertiesWidget : public QgsMapLayerConfigWidget, private Ui::QgsSurveyRendererPropertiesDialogBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsSurveyRendererPropertiesWidget, associated with the specified \a layer and \a style database.
     */
    QgsSurveyRendererPropertiesWidget( QgsSurveyLayer *layer, QgsStyle *style, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the \a context in which the widget is shown, e.g., the associated map canvas and expression contexts.
     */
    void setContext( const QgsSymbolWidgetContext &context );

    void syncToLayer( QgsMapLayer *layer ) final;

  public slots:

    void apply() override;

  private slots:
    void emitWidgetChanged();

  private:
    QgsSurveyLayer *mLayer = nullptr;
    QgsStyle *mStyle = nullptr;

    QgsMapCanvas *mMapCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;

    bool mBlockChangedSignal = false;
};


#endif // QGSSURVEYRENDERERPROPERTIESWIDGET_H
