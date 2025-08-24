#pragma once

#include "qgstaskmanager.h"
#include "qgssurveydata.h"

class QgsAbstractKnownPointsProvider;
class QgsSurveyDataProvider;

class QgsSurveyCoordinateApproximationTask : public QgsTask
{
    Q_OBJECT
  public:
    QgsSurveyCoordinateApproximationTask( QgsSurveyDataProvider *provider );
    ~QgsSurveyCoordinateApproximationTask();

    QHash<QString, QgsSurveyCoordinate> result() const { return mResult; }

  protected:
    bool run() override;

  private:
    QList<QgsSurveyObservationGroup> mObservations;
    QList<QgsSurveyPoint> mPoints;
    QgsAbstractKnownPointsProvider *mKnownPointsProvider;
    QgsCoordinateReferenceSystem mProjectCrs;
    QgsCoordinateReferenceSystem mDataCrs;
    QHash<QString, QgsSurveyCoordinate> mResult;
};
