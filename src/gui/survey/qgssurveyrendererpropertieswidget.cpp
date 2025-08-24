/***************************************************************************
    qgssurveyrendererpropertieswidget.cpp
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
#include "qgssurveyrendererpropertieswidget.h"
#include "moc_qgssurveyrendererpropertieswidget.cpp"

#include "qgis.h"
#include "qgssymbolwidgetcontext.h"
#include "qgssurveylayer.h"
#include "qgsproject.h"
#include "qgsprojectutils.h"

QgsSurveyRendererPropertiesWidget::QgsSurveyRendererPropertiesWidget( QgsSurveyLayer *layer, QgsStyle *style, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, nullptr, parent )
  , mLayer( layer )
  , mStyle( style )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );

  connect( mBlendModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSurveyRendererPropertiesWidget::emitWidgetChanged );
  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsSurveyRendererPropertiesWidget::emitWidgetChanged );

  mMaxErrorUnitWidget->setUnits( { Qgis::RenderUnit::Millimeters, Qgis::RenderUnit::MetersInMapUnits, Qgis::RenderUnit::MapUnits, Qgis::RenderUnit::Pixels, Qgis::RenderUnit::Points, Qgis::RenderUnit::Inches } );
  mMaxErrorSpinBox->setClearValue( 3 );

  connect( mMaxErrorSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsSurveyRendererPropertiesWidget::emitWidgetChanged );
  connect( mMaxErrorUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsSurveyRendererPropertiesWidget::emitWidgetChanged );

  syncToLayer( layer );
}

void QgsSurveyRendererPropertiesWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mMapCanvas = context.mapCanvas();
  mMessageBar = context.messageBar();
}

void QgsSurveyRendererPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = qobject_cast<QgsSurveyLayer *>( layer );

  mBlockChangedSignal = true;
  mOpacityWidget->setOpacity( mLayer->opacity() );
  mBlendModeComboBox->setShowClippingModes( QgsProjectUtils::layerIsContainedInGroupLayer( QgsProject::instance(), mLayer ) );
  mBlendModeComboBox->setBlendMode( mLayer->blendMode() );

  mBlockChangedSignal = false;
}

void QgsSurveyRendererPropertiesWidget::apply()
{
  mLayer->setOpacity( mOpacityWidget->opacity() );
  mLayer->setBlendMode( mBlendModeComboBox->blendMode() );
}

void QgsSurveyRendererPropertiesWidget::emitWidgetChanged()
{
  if ( !mBlockChangedSignal )
    emit widgetChanged();
}
