/***************************************************************************
                             qgstreewidget.h
                             -------------------
    begin                : 11 Jun, 2025
    copyright            : (C) Jan Dalheimer
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

#ifndef QGSTREEWIDGET_H
#define QGSTREEWIDGET_H

#include <QTreeWidget>
#include "qgis_gui.h"

class GUI_EXPORT QgsTreeWidgetMimeDataInterface
{
  public:
    virtual ~QgsTreeWidgetMimeDataInterface();

    virtual QMimeData *mimeData() const = 0;

    enum {
      HasMimeDataRole = Qt::UserRole + 5000,
    };
};

class GUI_EXPORT QgsTreeWidget : public QTreeWidget
{
    Q_OBJECT
  public:
    using QTreeWidget::QTreeWidget;

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    QMimeData *mimeData( const QList<QTreeWidgetItem *> &items ) const override;
#else
    QMimeData *mimeData(  QList<QTreeWidgetItem*> items ) const override;
#endif

    QStringList mimeTypes() const override;
    void setMimeTypes( const QStringList &types );

  private:
    QStringList mMimeTypes;
};

#endif // QGSTREEWIDGET_H
