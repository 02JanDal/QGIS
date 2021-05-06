/***************************************************************************
    qgsjsoneditwidget.cpp
     --------------------------------------
    Date                 : 3.5.2021
    Copyright            : (C) 2021 Damiano Lombardi
    Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsjsoneditwidget.h"

#include <QJsonArray>
#include <QPushButton>

QgsJsonEditWidget::QgsJsonEditWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  setView( View::Text );

  connect( mTextToolButton, &QToolButton::clicked, this, &QgsJsonEditWidget::textToolButtonClicked );
  connect( mTreeToolButton, &QToolButton::clicked, this, &QgsJsonEditWidget::treeToolButtonClicked );

  connect( mTextEdit, &QgsCodeEditorJson::textChanged, this, &QgsJsonEditWidget::refreshTreeView );
}

QgsJsonEditWidget::~QgsJsonEditWidget()
{

}

void QgsJsonEditWidget::setJsonText( const QString &jsonText )
{
  if ( mTextEdit->text() == jsonText )
    return;

  mTextEdit->setText( jsonText );

  refreshTreeView();
}

QString QgsJsonEditWidget::jsonText() const
{
  return mTextEdit->text();
}

bool QgsJsonEditWidget::validJson() const
{
  return true;
}

void QgsJsonEditWidget::setView( QgsJsonEditWidget::View view ) const
{
  switch ( view )
  {
    case View::Text:
    {
      mStackedWidget->setCurrentWidget( mStackedWidgetPageText );
      mTextToolButton->setChecked( true );
      mTreeToolButton->setChecked( false );
    }
    break;
    case View::Tree:
    {
      mStackedWidget->setCurrentWidget( mStackedWidgetPageTree );
      mTreeToolButton->setChecked( true );
      mTextToolButton->setChecked( false );
    }
    break;
  }
}

void QgsJsonEditWidget::textToolButtonClicked( bool checked )
{
  if ( checked )
    setView( View::Text );
  else
    setView( View::Tree );
}

void QgsJsonEditWidget::treeToolButtonClicked( bool checked )
{
  if ( checked )
    setView( View::Tree );
  else
    setView( View::Text );
}

void QgsJsonEditWidget::refreshTreeView()
{
  mTreeWidget->clear();

  if ( mTextEdit->text().isEmpty() )
    return;

  QJsonDocument jsonDocument = QJsonDocument::fromJson( mTextEdit->text().toUtf8() );

  if ( jsonDocument.isNull() )
  {
    setView( View::Text );
    mTextToolButton->setDisabled( true );
    mTreeToolButton->setDisabled( true );
    mTreeToolButton->setToolTip( tr( "Invalid JSON, tree view not available" ) );
    return;
  }
  mTextToolButton->setEnabled( true );
  mTreeToolButton->setEnabled( true );
  mTreeToolButton->setToolTip( tr( "Tree view" ) );

  if ( jsonDocument.isObject() )
  {
    for ( const QString &key : jsonDocument.object().keys() )
    {
      QJsonValue jsonValue = jsonDocument.object().value( key );
      QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem( mTreeWidget, QStringList() << key );
      refreshTreeViewItemValue( jsonValue, treeWidgetItem );
      mTreeWidget->addTopLevelItem( treeWidgetItem );
    }
  }
  else if ( jsonDocument.isArray() )
  {
    for ( int index = 0; index < jsonDocument.array().size(); index++ )
    {
      QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem( mTreeWidget, QStringList() << QString::number( index ) );
      refreshTreeViewItemValue( jsonDocument.array().at( index ), treeWidgetItem );
      mTreeWidget->addTopLevelItem( treeWidgetItem );
    }
  }
}

void QgsJsonEditWidget::refreshTreeViewItemValue( const QJsonValue &jsonValue, QTreeWidgetItem *treeWidgetItemParent )
{
  switch ( jsonValue.type() )
  {
    case QJsonValue::Null:
      treeWidgetItemParent->setText( ( int )TreeWidgetColumn::Value, QStringLiteral( "null" ) );
      break;
    case QJsonValue::Bool:
      treeWidgetItemParent->setText( ( int )TreeWidgetColumn::Value, jsonValue.toBool() ? "true" : "false" );
      break;
    case QJsonValue::Double:
      treeWidgetItemParent->setText( ( int )TreeWidgetColumn::Value, QString::number( jsonValue.toDouble() ) );
      break;
    case QJsonValue::String:
      treeWidgetItemParent->setText( ( int )TreeWidgetColumn::Value, jsonValue.toString() );
      break;
    case QJsonValue::Array:
    {
      QJsonArray jsonArray = jsonValue.toArray();
      for ( int index = 0; index < jsonArray.size(); index++ )
      {
        QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem( treeWidgetItemParent, QStringList() << QString::number( index ) );
        refreshTreeViewItemValue( jsonArray.at( index ), treeWidgetItem );
        treeWidgetItemParent->addChild( treeWidgetItem );
      }
    }
    break;
    case QJsonValue::Object:
    {
      QJsonObject jsonObject = jsonValue.toObject();
      for ( const QString &key : jsonObject.keys() )
      {
        QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem( treeWidgetItemParent, QStringList() << key );
        refreshTreeViewItemValue( jsonObject.value( key ), treeWidgetItem );
        treeWidgetItemParent->addChild( treeWidgetItem );
      }
    }
    break;
    case QJsonValue::Undefined:
      treeWidgetItemParent->setText( ( int )TreeWidgetColumn::Value, QStringLiteral( "Undefined value" ) );
      break;
  }
}

