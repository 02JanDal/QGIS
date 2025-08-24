/***************************************************************************
    qgssurveylayerstylewidget.cpp
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

#include "qgssurveylayerstylewidget.h"
#include "moc_qgssurveylayerstylewidget.cpp"
#include "qgssurveyrendererpropertieswidget.h"
#include "qgsstyle.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgssurveylayer.h"

QgsSurveyRendererWidgetFactory::QgsSurveyRendererWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/symbology.svg" ) ) );
  setTitle( tr( "Symbology" ) );
}

QgsMapLayerConfigWidget *QgsSurveyRendererWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *, bool, QWidget *parent ) const
{
  return new QgsSurveyRendererPropertiesWidget( qobject_cast<QgsSurveyLayer *>( layer ), QgsStyle::defaultStyle(), parent );
}

bool QgsSurveyRendererWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsSurveyRendererWidgetFactory::supportsStyleDock() const
{
  return true;
}

bool QgsSurveyRendererWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == Qgis::LayerType::Survey;
}

QString QgsSurveyRendererWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Rendering" );
}
