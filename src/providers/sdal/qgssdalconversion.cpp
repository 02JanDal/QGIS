#include "qgssdalconversion.h"

#include <QTimeZone>
#include <QtGlobal> // for QT_VERSION_CHECK

QgsSurveyPoint fromSdal( const sdal::Point &point, QgsSdalConversionContext &context )
{
  QHash<QgsSurveyCoordinateSource, QgsSurveyCoordinate> coordinates;
  for ( const auto &[k, v] : point.coordinates )
  {
    coordinates.insert( fromSdal( k, context ), fromSdal( v, context ) );
  }

  const QString name = fromSdal( point.name, context );
  return QgsSurveyPoint {
    .id = context.forPoint( name ),
    .name = name,
    .type = fromSdal<QgsSurveyPointType>( point.type, context ),
    .coordinates = coordinates,
    .attributes = fromSdal( point.attributes, context ),
    .metadata = fromSdal( sdal::AnyMetadata( point.metadata ), context )
  };
}

QgsSurveyPointType fromSdal( const sdal::PointType &type, QgsSdalConversionContext & )
{
  switch ( type )
  {
    case sdal::PointType::Fix:
      return QgsSurveyPointType::Fix;
    case sdal::PointType::New:
      return QgsSurveyPointType::New;
  }
}

QgsSurveyCoordinateSource fromSdal( const sdal::CoordinateSource &source, QgsSdalConversionContext & )
{
  switch ( source )
  {
    case sdal::CoordinateSource::KeyedIn:
      return QgsSurveyCoordinateSource::KeyedIn;
    case sdal::CoordinateSource::ReductionInInstrument:
      return QgsSurveyCoordinateSource::ReductionInInstrument;
    case sdal::CoordinateSource::Approximated:
      return QgsSurveyCoordinateSource::Approximated;
    case sdal::CoordinateSource::Adjusted:
      return QgsSurveyCoordinateSource::Adjusted;
  }
}

QString fromSdal( const std::optional<std::string> &string, QgsSdalConversionContext & )
{
  return string.has_value() ? QString::fromStdString( string.value() ) : QString();
}

QString fromSdal( const std::string &string, QgsSdalConversionContext & )
{
  return QString::fromStdString( string );
}

QgsSurveyMeasurementAngle fromSdal( const sdal::Angle &angle, QgsSdalConversionContext & )
{
  QgsSurveyMeasurementAngle output;
  switch ( angle.unit )
  {
    case sdal::Angle::Gon:
      output.unit = Qgis::AngleUnit::Gon;
      break;
    case sdal::Angle::Degrees:
      output.unit = Qgis::AngleUnit::Degrees;
      break;
    case sdal::Angle::Radians:
      output.unit = Qgis::AngleUnit::Radians;
      break;
  }
  output.value = angle.value;
  output.uncertainty = angle.uncertainty;
  return output;
}

QgsSurveyMeasurementDistance fromSdal( const sdal::Distance &distance, QgsSdalConversionContext &context )
{
  return QgsSurveyMeasurementDistance {
    .unit = fromSdal( distance.unit, context ),
    .value = distance.value,
    .uncertainty = distance.uncertainty
  };
}

QgsSurveyObservationType fromSdal( const sdal::ObservationType value, QgsSdalConversionContext & )
{
  switch ( value )
  {
    case sdal::ObservationType::BackSight:
      return QgsSurveyObservationType::BackSight;
    case sdal::ObservationType::Point:
      return QgsSurveyObservationType::Point;
    case sdal::ObservationType::RoundMean:
      return QgsSurveyObservationType::RoundMean;
    case sdal::ObservationType::StationMean:
      return QgsSurveyObservationType::StationMean;
  }
}

QgsSurveyObservation fromSdal( const sdal::Observation &value, QgsSdalConversionContext &context )
{
  return QgsSurveyObservation {
    .id = context.forObservation(),
    .at = fromSdal( value.at, context ),
    .point = context.forPoint( fromSdal( value.point, context ) ),
    .station = context.forPoint( fromSdal( value.station, context ) ),
    .type = fromSdal<QgsSurveyObservationType>( value.type, context ),
    .horizontalDistance = fromSdal<QgsSurveyMeasurementDistance>( value.horizontalDistance, context ),
    .heightDifference = fromSdal<QgsSurveyMeasurementDistance>( value.heightDifference, context ),
    .instrumentHeight = fromSdal<QgsSurveyMeasurementDistance>( value.instrumentHeight, context ),
    .horizontalDirection = fromSdal<QgsSurveyMeasurementAngle>( value.horizontalDirection, context ),
    .horizontalAzimuth = fromSdal<QgsSurveyMeasurementAngle>( value.horizontalAzimuth, context ),
    .zenithAngle = fromSdal<QgsSurveyMeasurementAngle>( value.zenithAngle, context ),
    .slopeDistance = fromSdal<QgsSurveyMeasurementDistance>( value.slopeDistance, context ),
    .reflectorHeight = fromSdal<QgsSurveyMeasurementDistance>( value.reflectorHeight, context ),
    .ppm = fromSdal<double>( value.ppm, context ),
    .prismConstant = fromSdal<double>( value.prismConstant, context ),
    .attributes = fromSdal( value.attributes, context ),
    .metadata = fromSdal( sdal::AnyMetadata( value.metadata ), context )
  };
}

QDateTime fromSdal( const std::optional<sdal::DateTime> &value, QgsSdalConversionContext &context )
{
  if ( value.has_value() )
  {
    return fromSdal( value.value(), context );
  }
  else
  {
    return {};
  }
}

QDateTime fromSdal( const sdal::DateTime &value, QgsSdalConversionContext & )
{
#if QT_VERSION >= QT_VERSION_CHECK( 6, 4, 0 )
  return QDateTime::fromStdTimePoint( value );
#else
  return QDateTime::fromMSecsSinceEpoch(
    std::chrono::duration_cast<std::chrono::milliseconds>( value.time_since_epoch() ).count(),
    QTimeZone::systemTimeZone()
  );
#endif
}

QgsSurveyObservationGroupAggregation fromSdal( const sdal::ObservationGroupAggregation value, QgsSdalConversionContext & )
{
  switch ( value )
  {
    case sdal::ObservationGroupAggregation::Rounds:
      return QgsSurveyObservationGroupAggregation::Rounds;
    case sdal::ObservationGroupAggregation::Leveling_BF:
      return QgsSurveyObservationGroupAggregation::Leveling_BF;
    case sdal::ObservationGroupAggregation::Leveling_BFFB:
      return QgsSurveyObservationGroupAggregation::Leveling_BFFB;
    case sdal::ObservationGroupAggregation::Leveling_aBF:
      return QgsSurveyObservationGroupAggregation::Leveling_aBF;
    case sdal::ObservationGroupAggregation::Leveling_aBFFB:
      return QgsSurveyObservationGroupAggregation::Leveling_aBFFB;
  }
}

QgsSurveyObservationGroup fromSdal( const sdal::ObservationGroup &value, QgsSdalConversionContext &context )
{
  return QgsSurveyObservationGroup {
    .id = context.forObservationGroup(),
    .station = context.forPoint( fromSdal( value.station, context ) ),
    .aggregation = fromSdal<QgsSurveyObservationGroupAggregation>( value.aggregation, context ),
    .observations = fromSdal<QgsSurveyObservation>( value.observations, context )
  };
}

QgsSurveyCoordinateXY fromSdal( const sdal::CoordinateXY &value, QgsSdalConversionContext &context )
{
  return QgsSurveyCoordinateXY {
    .unit = fromSdal( value.unit, context ),
    .x = value.x,
    .y = value.y,
    .uncertainty = value.uncertainty
  };
}

Qgis::DistanceUnit fromSdal( const sdal::Distance::Unit value, QgsSdalConversionContext & )
{
  switch ( value )
  {
    case sdal::Distance::Meters:
      return Qgis::DistanceUnit::Meters;
    case sdal::Distance::Millimeters:
      return Qgis::DistanceUnit::Millimeters;
    case sdal::Distance::Feet:
      return Qgis::DistanceUnit::Feet;
    case sdal::Distance::Inches:
      return Qgis::DistanceUnit::Inches;
  }
}

QgsSurveyCoordinate fromSdal( const sdal::Coordinate &value, QgsSdalConversionContext &context )
{
  return QgsSurveyCoordinate {
    .xy = fromSdal<QgsSurveyCoordinateXY>( value.xy, context ),
    .h = fromSdal<QgsSurveyMeasurementDistance>( value.h, context )
  };
}

QVariantHash fromSdal( const sdal::Attributes &value, QgsSdalConversionContext & )
{
  QVariantHash output;
  output.reserve( ( int ) value.size() );
  for ( const auto &[k, v] : value )
  {
    output.insert( QString::fromStdString( k ), std::visit( []( auto &&v ) {
                     using T = std::decay_t<decltype( v )>;
                     if constexpr ( std::is_same_v<std::string, T> )
                     {
                       return QVariant( QString::fromStdString( v ) );
                     }
                     else
                     {
                       return QVariant::fromValue( v );
                     }
                   },
                                                            v ) );
  }
  return output;
}

QVariantHash fromSdal( const sdal::AnyMetadata &value, QgsSdalConversionContext &context )
{
  QVariantHash output;
  output.reserve( ( int ) value.size() );
  for ( const auto &[k, v] : value )
  {
    output.insert( QString::fromStdString( k ), std::visit( [&context]( auto &&v ) {
                     using T = std::decay_t<decltype( v )>;
                     if constexpr ( std::is_same_v<std::string, T> )
                     {
                       return QVariant( QString::fromStdString( v ) );
                     }
                     else if constexpr ( std::is_same_v<sdal::Distance, T> || std::is_same_v<sdal::Angle, T> )
                     {
                       return QVariant::fromValue( fromSdal( v, context ) );
                     }
                     else
                     {
                       return QVariant::fromValue( v );
                     }
                   },
                                                            v ) );
  }
  return output;
}

QgsSurveyFieldBookEntry fromSdal( const sdal::FieldBookEntry &value, QgsSdalConversionContext &context )
{
  return QgsSurveyFieldBookEntry {
    .id = context.forEntry(),
    .at = fromSdal( value.at, context ),
    .observations = fromSdal<QgsSurveyObservationGroup>( value.observations, context ),
    .points = fromSdal<QgsSurveyPoint>( value.points, context ),
    .text = fromSdal( value.text, context ),
    .metadata = fromSdal( sdal::AnyMetadata( value.metadata ), context )
  };
}

QgsSdalConversionContext::QgsSdalConversionContext( const QHash<QString, QgsFeatureId> &pointIds )
  : mPointIds( pointIds )
{
  const QList<QgsFeatureId> ids = pointIds.values();
  Q_ASSERT_X( ids.size() == ids.toSet().size(), "QgsSdalConversionContext", "duplicate ids" );
  if ( !ids.isEmpty() )
  {
    mNext = *std::max_element( ids.cbegin(), ids.cend() ) + 1;
  }
}

QgsFeatureId QgsSdalConversionContext::next()
{
  return mNext;
}

QgsFeatureId QgsSdalConversionContext::forPoint( const QString &name )
{
  if ( name.isNull() )
  {
    return FID_NULL;
  }

  if ( mPointIds.contains( name ) )
  {
    return mPointIds.value( name );
  }
  else
  {
    const int id = mNext++;
    mPointIds.insert( name, id );
    return id;
  }
}

QgsFeatureId QgsSdalConversionContext::forObservation()
{
  return mNext++;
}

QgsFeatureId QgsSdalConversionContext::forObservationGroup()
{
  return mNext++;
}

QgsFeatureId QgsSdalConversionContext::forEntry()
{
  return mNext++;
}
