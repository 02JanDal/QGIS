#include "qgspointxy.h"
#include "qgssurveydata.h"
#include "qgsunittypes.h"

QPair<QgsSurveyCoordinateSource, QgsSurveyCoordinate> QgsSurveyPoint::bestCoordinate() const
{
  if ( auto it = coordinates.find( QgsSurveyCoordinateSource::Adjusted ); it != coordinates.cend() )
  {
    return qMakePair( it.key(), it.value() );
  }
  else if ( auto it = coordinates.find( QgsSurveyCoordinateSource::ReductionInInstrument ); it != coordinates.cend() )
  {
    return qMakePair( it.key(), it.value() );
  }
  else if ( auto it = coordinates.find( QgsSurveyCoordinateSource::KeyedIn ); it != coordinates.cend() )
  {
    return qMakePair( it.key(), it.value() );
  }
  else if ( auto it = coordinates.find( QgsSurveyCoordinateSource::Approximated ); it != coordinates.cend() )
  {
    return qMakePair( it.key(), it.value() );
  }
  else
  {
    throw std::runtime_error( "No coordinates available" );
  }
}

QgsPointXY QgsSurveyCoordinateXY::toPointXY() const
{
  return QgsPointXY( x, y );
}

QgsSurveyMeasurementAngle QgsSurveyMeasurementAngle::toUnit( const Qgis::AngleUnit unit ) const
{
  if ( unit == this->unit )
  {
    return *this;
  }

  const double factor = QgsUnitTypes::fromUnitToUnitFactor( this->unit, unit );
  return QgsSurveyMeasurementAngle {
    .unit = unit,
    .value = value * factor,
    .uncertainty = uncertainty.has_value() ? uncertainty.value() * factor : uncertainty
  };
}

QgsSurveyMeasurementDistance QgsSurveyMeasurementDistance::toUnit( const Qgis::DistanceUnit unit ) const
{
  if ( unit == this->unit )
  {
    return *this;
  }

  const double factor = QgsUnitTypes::fromUnitToUnitFactor( this->unit, unit );
  return QgsSurveyMeasurementDistance {
    .unit = unit,
    .value = value * factor,
    .uncertainty = uncertainty.has_value() ? uncertainty.value() * factor : uncertainty
  };
}
