/***************************************************************************
                             qgstreewidget.cpp
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

#include "qgstreewidget.h"
#include "qgstreewidgetitem.h"

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
QMimeData *QgsTreeWidget::mimeData(const QList<QTreeWidgetItem *> &items) const
#else
QMimeData *QgsTreeWidget::mimeData(const QList<QTreeWidgetItem*> items) const
#endif
{
  if ( !items.isEmpty() && items.first()->data( 0, QgsTreeWidgetMimeDataInterface::HasMimeDataRole ).toBool() )
  {
    QgsTreeWidgetMimeDataInterface *item = reinterpret_cast<QgsTreeWidgetMimeDataInterface *>( items.first() );
    return item->mimeData();
  }
  return QTreeWidget::mimeData( items );
}

QStringList QgsTreeWidget::mimeTypes() const
{
  return mMimeTypes;
}
void QgsTreeWidget::setMimeTypes(const QStringList &types)
{
  mMimeTypes = types;
}

QgsTreeWidgetMimeDataInterface::~QgsTreeWidgetMimeDataInterface()
{
}
