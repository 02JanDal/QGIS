#include "qgscoordinatetransform.h"
#include "qgsproject.h"
#include "qgsprojectsurveyproperties.h"
#include "qgssurveycoordinateapproximationtask.h"

#include "qgssurveyadjustment.h"
#include "qgssurveydataprovider.h"

QgsSurveyCoordinateApproximationTask::QgsSurveyCoordinateApproximationTask( QgsSurveyDataProvider *provider )
  : QgsTask( tr( "Approximating coordinates from survey observations" ) ), mObservations( provider->observationGroups() ), mPoints( provider->points() ), mKnownPointsProvider( QgsProject::instance()->surveyProperties()->knownPointsProvider()->clone() ), mProjectCrs( QgsProject::instance()->crs() ), mDataCrs( provider->crs() )
{}

QgsSurveyCoordinateApproximationTask::~QgsSurveyCoordinateApproximationTask()
{
}

bool QgsSurveyCoordinateApproximationTask::run()
{
  QgsSurveyAdjustment adjustment;
  adjustment.addPoints( mPoints );
  adjustment.addObservations( mObservations );

  const QgsCoordinateReferenceSystem dataCrs = mDataCrs.isValid() ? mDataCrs : mProjectCrs;

  QHash<QgsFeatureId, QString> idToName;
  for ( const QgsSurveyPoint &p : mPoints )
  {
    idToName.insert( p.id, p.name );
  }
  QSet<QString> pointNames;
  for ( const QgsFeatureId &id : adjustment.referencedPoints() )
  {
    pointNames.insert( idToName.value( id ) );
  }

  QHash<QString, QgsPoint> knownPoints = mKnownPointsProvider->points( pointNames );
  if ( mKnownPointsProvider->crs() != dataCrs )
  {
    const QgsCoordinateTransform transform( mKnownPointsProvider->crs(), dataCrs, QgsProject::instance() );
    for ( auto &point : knownPoints )
    {
      point.transform( transform );
    }
  }
  QHash<QgsFeatureId, QgsPoint> knownPointsById;
  for ( auto it = knownPoints.cbegin(); it != knownPoints.cend(); ++it )
  {
    knownPointsById.insert( idToName.key( it.key() ), it.value() );
  }
  adjustment.addKnownPoints( knownPointsById );

  adjustment.approximate();
  mResult = adjustment.results();

  return true;
}
