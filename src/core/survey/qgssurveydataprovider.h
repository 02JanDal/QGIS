/***************************************************************************
  qgssurveydataprovider.h - QgsSurveyDataProvider

 ---------------------
 begin                : 2025-07-23
 copyright            : (C) 2025 by Jan Dalheimer
 email                : jan <at> dalheimer <dot> de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSURVEYDATAPROVIDER_H
#define QGSSURVEYDATAPROVIDER_H

#include "qgsdataprovider.h"
#include "qgssurveydatainterface.h"
#include "qgssurveycoordinateapproximationtask.h"

class CORE_EXPORT QgsSurveyDataProvider : public QgsDataProvider, public QgsSurveyDataInterface
{
    Q_OBJECT
  public:
    enum SurveyCoordinateApproximationState SIP_ENUM_BASETYPE( IntFlag )
    {
      NotApproximated, //!< Some coordinates might not have any coordinates
      Approximating,   //!< Provider is approximating coordinates
      Approximated     //!< Coordinates have been approximated where possible
    };

    QgsSurveyDataProvider(const QString &uri = QString(),
                           const QgsDataProvider::ProviderOptions &providerOptions = QgsDataProvider::ProviderOptions(),
                           Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags());

    virtual Qgis::SurveyProviderCapabilities capabilities() const;
    QgsRectangle extent() const override;

    virtual QgsSurveyObservation observation( const QgsFeatureId id ) const = 0;

    virtual std::variant<QgsSurveyFieldBookEntry, QgsSurveyPoint, QgsSurveyObservationGroup, QgsSurveyObservation> item( const QgsFeatureId id ) const = 0;

    virtual QgsSurveyDataProvider *clone() const = 0 SIP_FACTORY;

    SurveyCoordinateApproximationState approximationState() const;
    void startApproximating();

  signals:
    void approximationStateChanged();

  private:
    QgsSurveyCoordinateApproximationTask *mApproximatingTask = nullptr;
    QHash<QString, QgsSurveyCoordinate> mApproximatedCoordinates;
    bool mApproximationFinished = false;

  protected:
    QList<QgsSurveyPoint> pointsWithApproximations( const QList<QgsSurveyPoint> &points ) const;
    QgsSurveyPoint pointWithApproximation( const QgsSurveyPoint &point ) const;

    explicit QgsSurveyDataProvider( QgsSurveyDataProvider *other );
};

#endif // QGSSURVEYDATAPROVIDER_H
