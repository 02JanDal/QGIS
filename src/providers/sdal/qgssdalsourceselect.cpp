/***************************************************************************
  qgssdalsourceselect.cpp
  -------------------
    begin                : 2025-07-24
    copyright            : (C) 2025 by Jan Dalheimer
    email                : jan at dalheimer dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssdalsourceselect.h"
#include "moc_qgssdalsourceselect.cpp"
#include "qgsproviderregistry.h"
#include "qgshelp.h"

#include <QMessageBox>


QgsSdalSourceSelect::QgsSdalSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  setupButtons( buttonBox );

  mFileWidget->setDialogTitle( tr( "Open SDAL Dataset" ) );
  mFileWidget->setFilter( QgsProviderRegistry::instance()->fileSurveyFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetFile );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [this]() {
    emit enableButtons( !mFileWidget->filePath().isEmpty() );
  } );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsSdalSourceSelect::showHelp );

  connect( mAddBtn, &QPushButton::clicked, this, [this]() {
    mOptionsWidget->insertRow( mOptionsWidget->rowCount() );
  } );
  connect( mRemoveBtn, &QPushButton::clicked, this, [this]() {
    mOptionsWidget->removeRow( mOptionsWidget->currentRow() );
  } );
  mRemoveBtn->setEnabled( false );
  connect( mOptionsWidget, &QTableWidget::currentItemChanged, this, [this]( QTableWidgetItem *item ) {
    mRemoveBtn->setEnabled( !!item );
  } );
  connect( mOptionsWidget, &QTableWidget::itemChanged, this, []( QTableWidgetItem *item ) {
    if ( item->column() == 0 )
    {
      item->setData( Qt::EditRole, item->data( Qt::EditRole ).toString().toUpper() );
    }
  } );
}

void QgsSdalSourceSelect::addButtonClicked()
{
  if ( mFileWidget->filePath().isEmpty() )
  {
    QMessageBox::information( this, tr( "Add SDAL Layer" ), tr( "No layers selected." ) );
    return;
  }

  const QFileInfo fileInfo( mFileWidget->filePath() );
  if ( !fileInfo.isReadable() )
  {
    QMessageBox::warning( nullptr, tr( "Add SDAL Layer" ), tr( "Unable to read the selected file.\n"
                                                              "Please select a valid file." ) );
    return;
  }

  QStringList openOptions;
  for ( int i = 0; i < mOptionsWidget->rowCount(); ++i )
  {
    openOptions.append( QStringLiteral( "%1=%2" ).arg(
      mOptionsWidget->item( i, 0 )->data( Qt::EditRole ).toString(),
      mOptionsWidget->item( i, 1 )->data( Qt::EditRole ).toString()
    ) );
  }
  QVariantMap parts;
  if ( !openOptions.isEmpty() )
  {
    parts.insert( "openOptions", openOptions );
  }
  parts.insert( "path", fileInfo.absoluteFilePath() );

  emit addLayer( Qgis::LayerType::Survey, QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "sdal" ), parts ), fileInfo.baseName(), QStringLiteral( "sdal" ) );
}

void QgsSdalSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#survey" ) );
}
