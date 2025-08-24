/***************************************************************************
    qgsprojectsurveysettingswidget.h
    ---------------------
    begin                : August 2025
    copyright            : (C) 2025 by Jan Dalheimer
    email                : jan at dalheimer dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROJECTSURVEYSETTINGSWIDGET_H
#define QGSPROJECTSURVEYSETTINGSWIDGET_H

#include "qgsoptionswidgetfactory.h"

#include "ui_qgsprojectsurveysettingswidgetbase.h"

class QgsProjectSurveySettingsWidget : public QgsOptionsPageWidget, private Ui::QgsProjectSurveySettingsWidgetBase
{
    Q_OBJECT
  public:
    QgsProjectSurveySettingsWidget( QWidget *parent = nullptr );

  public slots:
    bool isValid() override;
    void apply() override;

  private slots:

    bool validate();
};


class QgsProjectSurveySettingsWidgetFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsProjectSurveySettingsWidgetFactory( QObject *parent = nullptr );

    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
};


#endif // QGSPROJECTSURVEYSETTINGSWIDGET_H
