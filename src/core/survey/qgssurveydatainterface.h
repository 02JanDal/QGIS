
/***************************************************************************
  qgssurveydatainterface.h - QgsSurveyDataInterface

 ---------------------
 begin                : 23.7.2025
 copyright            : (C) 2025 by jan
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSURVEYDATAINTERFACE_H
#define QGSSURVEYDATAINTERFACE_H

#include "qgsfeatureid.h"
#include "qgssurveydata.h"

class CORE_EXPORT QgsSurveyDataInterface
{
  public:
    QgsSurveyDataInterface();
    virtual ~QgsSurveyDataInterface();

    virtual QList<QgsSurveyFieldBookEntry> fieldBook() const = 0;
    virtual QgsSurveyFieldBookEntry fieldBookEntry( const QgsFeatureId id ) const = 0;
    virtual int fieldBookCount() const = 0;
    virtual QList<QgsSurveyPoint> points() const = 0;
    virtual QgsSurveyPoint point( const QgsFeatureId id ) const = 0;
    virtual int pointCount() const = 0;
    virtual QList<QgsSurveyObservationGroup> observationGroups() const = 0;
    virtual QgsSurveyObservationGroup observationGroup( const QgsFeatureId id ) const = 0;
    virtual int observationGroupsCount() const = 0;
};

#endif // QGSSURVEYDATAINTERFACE_H
