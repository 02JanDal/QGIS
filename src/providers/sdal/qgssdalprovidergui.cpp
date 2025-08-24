/***************************************************************************
  qgssdalprovidergui.cpp
  --------------------------------------
  Date                 : 2025-07-23
  Copyright            : (C) 2025 by Jan Dalheimer
  Email                : jan at dalheimer dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproviderguimetadata.h"
#include "qgssourceselectprovider.h"

#include "qgssdalsourceselect.h"


//! Provider for SDAL source select
class QgsSdalSourceSelectProvider : public QgsSourceSelectProvider
{
  public:
    QString providerKey() const override { return QStringLiteral( "sdal" ); }
    QString text() const override { return QObject::tr( "Survey" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderLocalProvider + 70; }
    QIcon icon() const override { return QIcon(); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsSdalSourceSelect( parent, fl, widgetMode );
    }
};


class QgsSdalProviderGuiMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsSdalProviderGuiMetadata()
      : QgsProviderGuiMetadata( QStringLiteral( "sdal" ) )
    {
    }

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override
    {
      QList<QgsSourceSelectProvider *> providers;
      providers << new QgsSdalSourceSelectProvider;
      return providers;
    }
};


QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsSdalProviderGuiMetadata();
}
