/***************************************************************************
    qgsprojectsurveysettingswidget.cpp
    ---------------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectsurveysettingswidget.h"
#include "moc_qgsprojectsurveysettingswidget.cpp"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsprojectsurveyproperties.h"
#include "qgsvectorlayer.h"

QgsProjectSurveySettingsWidget::QgsProjectSurveySettingsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  mComboKnownPointsType->addItem( tr( "None" ), QStringLiteral( "no" ) );
  mComboKnownPointsType->addItem( tr( "Vector Layer" ), QStringLiteral( "vector" ) );

  mComboVectorLayer->setFilters( Qgis::LayerFilter::PointLayer );

  mStackedWidget->setSizeMode( QgsStackedWidget::SizeMode::CurrentPageOnly );

  mStackedWidget->setCurrentWidget( mPageNo );
  connect( mComboKnownPointsType, qOverload<int>( &QComboBox::currentIndexChanged ), this, [this] {
    const QString knownPointsType = mComboKnownPointsType->currentData().toString();
    if ( knownPointsType == QLatin1String( "no" ) )
    {
      mStackedWidget->setCurrentWidget( mPageNo );
    }
    else if ( knownPointsType == QLatin1String( "vector" ) )
    {
      mStackedWidget->setCurrentWidget( mPageVector );
    }
    validate();
  } );
  connect( mComboVectorLayer, &QgsMapLayerComboBox::layerChanged, mComboVectorField, &QgsFieldComboBox::setLayer );

  // setup with current settings
  QgsProjectSurveyProperties *surveyProperties = QgsProject::instance()->surveyProperties();
  const QgsAbstractKnownPointsProvider *provider = surveyProperties->knownPointsProvider();
  mComboKnownPointsType->setCurrentIndex( mComboKnownPointsType->findData( provider->type() ) );
  if ( provider->type() == QLatin1String( "vector" ) )
  {
    mStackedWidget->setCurrentWidget( mPageVector );
    mComboVectorLayer->setLayer( qgis::down_cast<const QgsVectorKnownPointsProvider *>( provider )->layer() );
    mComboVectorField->setLayer( mComboVectorLayer->currentLayer() );
    mComboVectorField->setField( qgis::down_cast<const QgsVectorKnownPointsProvider *>( provider )->keyAttribute() );
  }

  connect( mComboVectorLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsProjectSurveySettingsWidget::validate );
  connect( mComboVectorField, &QgsFieldComboBox::fieldChanged, this, &QgsProjectSurveySettingsWidget::validate );

  if ( mComboVectorLayer->currentLayer() )
    mComboVectorField->setLayer( mComboVectorLayer->currentLayer() );

  validate();
}

void QgsProjectSurveySettingsWidget::apply()
{
  const QString knownPointsType = mComboKnownPointsType->currentData().toString();
  std::unique_ptr<QgsAbstractKnownPointsProvider> provider;
  if ( knownPointsType == QLatin1String( "no" ) )
  {
    provider = std::make_unique<QgsNoKnownPointsProvider>();
  }
  else if ( knownPointsType == QLatin1String( "vector" ) )
  {
    provider = std::make_unique<QgsVectorKnownPointsProvider>();
    QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mComboVectorLayer->currentLayer() );
    qgis::down_cast<QgsVectorKnownPointsProvider *>( provider.get() )->setLayer( layer );
    qgis::down_cast<QgsVectorKnownPointsProvider *>( provider.get() )->setKeyAttribute( mComboVectorField->currentField() );
  }

  QgsProject::instance()->surveyProperties()->setKnownPointsProvider( provider.release() );
}

bool QgsProjectSurveySettingsWidget::validate()
{
  mMessageBar->clearWidgets();

  bool valid = true;
  const QString knownPointsType = mComboKnownPointsType->currentData().toString();
  if ( knownPointsType == QLatin1String( "vector" ) )
  {
    if ( !mComboVectorLayer->currentLayer() )
    {
      valid = false;
      mMessageBar->pushMessage( tr( "An survey layer must be selected" ), Qgis::MessageLevel::Critical );
    }
  }

  return valid;
}

bool QgsProjectSurveySettingsWidget::isValid()
{
  return validate();
}


//
// QgsProjectSurveySettingsWidgetFactory
//

QgsProjectSurveySettingsWidgetFactory::QgsProjectSurveySettingsWidgetFactory( QObject *parent )
  : QgsOptionsWidgetFactory( tr( "Survey" ), QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/surveyscale.svg" ) ), QStringLiteral( "terrain" ) )
{
  setParent( parent );
}


QgsOptionsPageWidget *QgsProjectSurveySettingsWidgetFactory::createWidget( QWidget *parent ) const
{
  return new QgsProjectSurveySettingsWidget( parent );
}
