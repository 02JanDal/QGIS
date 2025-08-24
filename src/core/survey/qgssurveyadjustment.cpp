#include "qgssurveyadjustment.h"

#include <gnu_gama/local/acord/acord2.h>

#include <QSet>
#include <qgsunittypes.h>

struct QgsSurveyAdjustmentPrivate
{
    GNU_gama::local::PointData pointData;
    GNU_gama::local::ObservationData observationData;
};

QgsSurveyAdjustment::QgsSurveyAdjustment()
  : mPrivate( std::make_unique<QgsSurveyAdjustmentPrivate>() ) {}

QgsSurveyAdjustment::~QgsSurveyAdjustment() = default;

void QgsSurveyAdjustment::addPoints( const QList<QgsSurveyPoint> &points )
{
  for ( const QgsSurveyPoint &point : points )
  {
    mReferencedPoints.insert( point.id );

    GNU_gama::local::LocalPoint p;
    if ( point.coordinates.contains( QgsSurveyCoordinateSource::KeyedIn ) )
    {
      const QgsSurveyCoordinate c = point.coordinates.value( QgsSurveyCoordinateSource::KeyedIn );
      const bool isFix = point.type.has_value() && point.type.value() == QgsSurveyPointType::Fix;
      if ( c.xy.has_value() )
      {
        p.set_xy( c.xy->x, c.xy->y );
        if ( isFix )
        {
          p.set_fixed_xy();
        }
        else
        {
          p.set_free_xy();
        }
      }
      if ( c.h.has_value() )
      {
        p.set_z( c.h->value );
        if ( isFix )
        {
          p.set_fixed_z();
        }
        else
        {
          p.set_free_z();
        }
      }
    }
    else
    {
      p.set_free_xy();
      p.set_free_z();
    }

    mPrivate->pointData.insert( { point.name.toStdString(), p } );
  }
}
void QgsSurveyAdjustment::addObservations( const QList<QgsSurveyObservationGroup> &observations )
{
  QgsFeatureIds pointsReferredToByObservations;
  for ( const QgsSurveyObservationGroup &og : observations )
  {
    if ( og.station != FID_NULL )
    {
      pointsReferredToByObservations.insert( og.station );
    }
    for ( const QgsSurveyObservation &o : og.observations )
    {
      if ( og.station != FID_NULL )
      {
        pointsReferredToByObservations.insert( o.station );
      }
      pointsReferredToByObservations.insert( o.point );
    }
  }
  mReferencedPoints.unite( pointsReferredToByObservations );
  for ( const QgsFeatureId &point : pointsReferredToByObservations )
  {
    if ( mPrivate->pointData.count( std::to_string( point ) ) == 0 )
    {
      GNU_gama::local::LocalPoint p;
      p.set_free_xy();
      p.set_free_z();
      mPrivate->pointData.insert( { std::to_string( point ), p } );
    }
  }

  for ( const QgsSurveyObservationGroup &og : observations )
  {
    GNU_gama::local::StandPoint *point = new GNU_gama::local::StandPoint( &mPrivate->observationData );
    if ( og.station != FID_NULL )
    {
      point->station = std::to_string( og.station );
    }
    mPrivate->observationData.clusters.push_back( point );
    for ( const QgsSurveyObservation &o : og.observations )
    {
      if ( o.station != FID_NULL )
      {
        if ( o.slopeDistance.has_value() )
        {
          point->observation_list.push_back(
            new GNU_gama::local::S_Distance( std::to_string( o.station ), std::to_string( o.point ), toMeters( o.slopeDistance.value() ) )
          );
        }
        if ( o.zenithAngle.has_value() )
        {
          point->observation_list.push_back(
            new GNU_gama::local::Z_Angle( std::to_string( o.station ), std::to_string( o.point ), toRadians( o.zenithAngle.value() ) )
          );
        }
        if ( o.horizontalDirection.has_value() )
        {
          point->observation_list.push_back( new GNU_gama::local::Direction( std::to_string( o.station ), std::to_string( o.point ), toRadians( o.horizontalDirection.value() ) ) );
        }
      }
    }
    for ( GNU_gama::local::Observation *obs : point->observation_list )
    {
      obs->set_cluster( point );
    }
  }
}

void QgsSurveyAdjustment::addKnownPoints( const QHash<QgsFeatureId, QgsPoint> &points )
{
  for ( auto it = points.cbegin(); it != points.cend(); ++it )
  {
    const std::string name = std::to_string( it.key() );
    GNU_gama::local::PointData::iterator pointIt = mPrivate->pointData.find( name );
    if ( pointIt == mPrivate->pointData.end() )
    {
      pointIt = mPrivate->pointData.insert( { name, GNU_gama::local::LocalPoint() } ).first;
    }
    pointIt->second.set_xy( it.value().x(), it.value().y() );
    pointIt->second.set_fixed_xy();
    if ( it.value().is3D() )
    {
      pointIt->second.set_z( it.value().z() );
      pointIt->second.set_fixed_z();
    }
  }
}

void QgsSurveyAdjustment::approximate()
{
  GNU_gama::local::Acord2( mPrivate->pointData, mPrivate->observationData ).execute();
}

QHash<QString, QgsSurveyCoordinate> QgsSurveyAdjustment::results() const
{
  QHash<QString, QgsSurveyCoordinate> results;

  for ( const auto &[k, v] : mPrivate->pointData )
  {
    QgsSurveyCoordinate coordinate;
    if ( v.test_xy() )
    {
      coordinate.xy = QgsSurveyCoordinateXY {
        .unit = Qgis::DistanceUnit::Meters,
        .x = v.x(),
        .y = v.y(),
        .uncertainty = {},
      };
    }
    if ( v.test_z() )
    {
      coordinate.h = QgsSurveyMeasurementDistance {
        .unit = Qgis::DistanceUnit::Meters,
        .value = v.z(),
        .uncertainty = {},
      };
    }
    results.insert( QString::fromStdString( k.str() ), coordinate );
  }

  return results;
}

double QgsSurveyAdjustment::toMeters( const QgsSurveyMeasurementDistance &distance )
{
  return distance.value * QgsUnitTypes::fromUnitToUnitFactor( distance.unit, Qgis::DistanceUnit::Meters );
}
double QgsSurveyAdjustment::toRadians( const QgsSurveyMeasurementAngle &angle )
{
  return angle.value * QgsUnitTypes::fromUnitToUnitFactor( angle.unit, Qgis::AngleUnit::Radians );
}
