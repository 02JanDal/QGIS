#pragma once

#include <QDateTime>
#include <optional>

#include "qgis.h"
#include "qgsfeatureid.h"

class QgsPointXY;

/// Mirrors sdal::Angle
struct CORE_EXPORT QgsSurveyMeasurementAngle
{
    Qgis::AngleUnit unit;
    double value;
    std::optional<double> uncertainty;

    QgsSurveyMeasurementAngle toUnit( const Qgis::AngleUnit unit ) const;
};
Q_DECLARE_METATYPE( QgsSurveyMeasurementAngle )
/// Mirrors sdal::Angle
struct CORE_EXPORT QgsSurveyMeasurementDistance
{
    Qgis::DistanceUnit unit;
    double value;
    std::optional<double> uncertainty;

    QgsSurveyMeasurementDistance toUnit( const Qgis::DistanceUnit unit ) const;
};
Q_DECLARE_METATYPE( QgsSurveyMeasurementDistance )
/// Mirrors sdal::ObservationType
enum class QgsSurveyObservationType
{
  BackSight,
  Point,
  RoundMean,
  StationMean,
};
Q_DECLARE_METATYPE( QgsSurveyObservationType )
/// Mirrors sdal::Observation
struct QgsSurveyObservation
{
    QgsFeatureId id;

    QDateTime at;
    QgsFeatureId point;
    QgsFeatureId station;
    std::optional<QgsSurveyObservationType> type;

    std::optional<QgsSurveyMeasurementDistance> horizontalDistance;
    std::optional<QgsSurveyMeasurementDistance> heightDifference;
    std::optional<QgsSurveyMeasurementDistance> instrumentHeight;

    std::optional<QgsSurveyMeasurementAngle> horizontalDirection;
    std::optional<QgsSurveyMeasurementAngle> horizontalAzimuth;
    std::optional<QgsSurveyMeasurementAngle> zenithAngle;
    std::optional<QgsSurveyMeasurementDistance> slopeDistance;
    std::optional<QgsSurveyMeasurementDistance> reflectorHeight;
    std::optional<double> ppm;
    std::optional<double> prismConstant;

    QVariantHash attributes;
    QVariantHash metadata;
};
/// Mirrors sdal::ObservationGroupAggregation
enum class QgsSurveyObservationGroupAggregation
{
  Rounds,
  Leveling_BF,
  Leveling_BFFB,
  Leveling_aBF,
  Leveling_aBFFB
};
Q_DECLARE_METATYPE( QgsSurveyObservationGroupAggregation )
/// Mirrors sdal::ObservationGroup
struct QgsSurveyObservationGroup
{
    QgsFeatureId id;

    QgsFeatureId station;
    std::optional<QgsSurveyObservationGroupAggregation> aggregation;
    QVector<QgsSurveyObservation> observations;
};

struct QgsSurveyCoordinateXY
{
    Qgis::DistanceUnit unit;
    double x, y;
    std::optional<double> uncertainty;

    QgsPointXY toPointXY() const;
};
struct QgsSurveyCoordinate
{
    std::optional<QgsSurveyCoordinateXY> xy;
    std::optional<QgsSurveyMeasurementDistance> h;
};

/// Mirrors sdal::CoordinateSource
enum class QgsSurveyCoordinateSource
{
  KeyedIn,
  ReductionInInstrument,
  Approximated,
  Adjusted,
};
QHASH_FOR_CLASS_ENUM( QgsSurveyCoordinateSource )
/// Mirrors sdal::PointType
enum class QgsSurveyPointType
{
  Fix,
  New
};
Q_DECLARE_METATYPE( QgsSurveyPointType )
/// Mirrors sdal::Point
struct CORE_EXPORT QgsSurveyPoint
{
    QgsFeatureId id;

    QString name;
    std::optional<QgsSurveyPointType> type;
    QHash<QgsSurveyCoordinateSource, QgsSurveyCoordinate> coordinates;

    QVariantHash attributes;
    QVariantHash metadata;

    QPair<QgsSurveyCoordinateSource, QgsSurveyCoordinate> bestCoordinate() const;
};

/// Mirrors sdal::FieldBookEntry
struct QgsSurveyFieldBookEntry
{
    QgsFeatureId id;

    QDateTime at;
    std::optional<QgsSurveyObservationGroup> observations;
    QVector<QgsSurveyPoint> points;
    QString text;

    QVariantHash metadata;
};
