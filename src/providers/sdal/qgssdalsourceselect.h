/***************************************************************************
  qgssdalsourceselect.h
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

#ifndef QGSSDALSOURCESELECT_H
#define QGSSDALSOURCESELECT_H

#include "ui_qgssdalsourceselectbase.h"
#include "qgsabstractdatasourcewidget.h"


/**
 * \class QgsSdalSourceSelect
 * \brief Dialog to select SDAL supported sources
 */
class QgsSdalSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsSdalSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsSdalSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Standalone );

  public slots:
    void addButtonClicked() override;

  private slots:
    void showHelp();
};

#endif // QGSSDALSOURCESELECT_H
