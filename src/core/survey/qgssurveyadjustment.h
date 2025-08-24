#pragma once

#include "qgspoint.h"
#include "qgssurveydata.h"

#include <QSet>

struct QgsSurveyAdjustmentPrivate;

class QgsSurveyAdjustment
{
  public:
    explicit QgsSurveyAdjustment();
    ~QgsSurveyAdjustment();

    void addPoints( const QList<QgsSurveyPoint> &points );
    void addObservations( const QList<QgsSurveyObservationGroup> &observations );
    void addKnownPoints( const QHash<QgsFeatureId, QgsPoint> &points );

    QgsFeatureIds referencedPoints() const { return mReferencedPoints; }

    void approximate();

    QHash<QString, QgsSurveyCoordinate> results() const;

  private:
    std::unique_ptr<QgsSurveyAdjustmentPrivate> mPrivate;
    QgsFeatureIds mReferencedPoints;

    static double toMeters( const QgsSurveyMeasurementDistance &distance );
    static double toRadians( const QgsSurveyMeasurementAngle &angle );
};
