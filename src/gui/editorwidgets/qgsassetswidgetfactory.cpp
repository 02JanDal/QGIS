/***************************************************************************
   qgsassetswidgetfactory.cpp

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

#include "qgsassetswidgetfactory.h"
#include "qgsassetswidgetwrapper.h"
#include "qgsdummyconfigdlg.h"
#include "qgsfeatureasset.h"

QgsAssetsWidgetFactory::QgsAssetsWidgetFactory( const QString &name, QgsMessageBar *messageBar )
  : QgsEditorWidgetFactory( name )
  , mMessageBar( messageBar )
{
}

QgsEditorWidgetWrapper *QgsAssetsWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsAssetsWidgetWrapper( vl, fieldIdx, editor, mMessageBar, parent );
}

QgsEditorConfigWidget *QgsAssetsWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsDummyConfigDlg( vl, fieldIdx, parent, QObject::tr( "" ) );
}

unsigned int QgsAssetsWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  const auto type = vl->fields().at( fieldIdx ).type();
  const auto subType = vl->fields().at( fieldIdx ).subType();
  if ( type == QMetaType::Type::QVariantMap && subType == qMetaTypeId<QgsFeatureAsset>() )
    return 30;

  return 0;
}
