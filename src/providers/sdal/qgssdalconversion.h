#pragma once

#include "qgssurveydata.h"
#include <sdal/data/Basic.h>
#include <sdal/data/Point.h>
#include <sdal/data/Observation.h>
#include <sdal/data/FieldBook.h>

class CORE_EXPORT QgsSdalConversionContext
{
  public:
    explicit QgsSdalConversionContext(
      const QHash<QString, QgsFeatureId> &pointIds
    );

    QgsFeatureId next();

    QgsFeatureId forPoint( const QString &name );
    QgsFeatureId forObservation();
    QgsFeatureId forObservationGroup();
    QgsFeatureId forEntry();

  private:
    QgsFeatureId mNext = 0;

    QHash<QString, QgsFeatureId> mPointIds;
};

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
T fromSdal( const T v, QgsSdalConversionContext & ) { return v; }

QString fromSdal( const std::optional<std::string> &string, QgsSdalConversionContext & );
QString fromSdal( const std::string &string, QgsSdalConversionContext & );
QDateTime fromSdal( const std::optional<sdal::DateTime> &value, QgsSdalConversionContext & );
QDateTime fromSdal( const sdal::DateTime &value, QgsSdalConversionContext & );
QVariantHash fromSdal( const sdal::Attributes &value, QgsSdalConversionContext & );
QVariantHash fromSdal( const sdal::AnyMetadata &value, QgsSdalConversionContext & );

QgsSurveyMeasurementAngle fromSdal( const sdal::Angle &angle, QgsSdalConversionContext & );
QgsSurveyMeasurementDistance fromSdal( const sdal::Distance &distance, QgsSdalConversionContext & );
Qgis::DistanceUnit fromSdal( const sdal::Distance::Unit value, QgsSdalConversionContext & );
QgsSurveyObservationType fromSdal( const sdal::ObservationType value, QgsSdalConversionContext & );
QgsSurveyObservation fromSdal( const sdal::Observation &value, QgsSdalConversionContext &context );
QgsSurveyObservationGroupAggregation fromSdal( const sdal::ObservationGroupAggregation value, QgsSdalConversionContext & );
QgsSurveyObservationGroup fromSdal( const sdal::ObservationGroup &value, QgsSdalConversionContext &context );

QgsSurveyCoordinateXY fromSdal( const sdal::CoordinateXY &value, QgsSdalConversionContext & );
QgsSurveyCoordinate fromSdal( const sdal::Coordinate &value, QgsSdalConversionContext & );
QgsSurveyCoordinateSource fromSdal( const sdal::CoordinateSource &source, QgsSdalConversionContext & );
QgsSurveyPointType fromSdal( const sdal::PointType &type, QgsSdalConversionContext & );
QgsSurveyPoint fromSdal( const sdal::Point &point, QgsSdalConversionContext &context );

QgsSurveyFieldBookEntry fromSdal( const sdal::FieldBookEntry &value, QgsSdalConversionContext &context );

template<typename A, typename B>
std::optional<A> fromSdal( const std::optional<B> &value, QgsSdalConversionContext &context )
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
template<typename A, typename B>
QVector<A> fromSdal( const std::vector<B> &value, QgsSdalConversionContext &context )
{
  QVector<A> result;
  result.reserve( value.size() );
  std::transform( value.cbegin(), value.cend(), std::back_inserter( result ), [&context]( auto &&v ) { return fromSdal( v, context ); } );
  return result;
}
