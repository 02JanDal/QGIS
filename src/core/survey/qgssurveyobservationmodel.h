#pragma once

#include "qgsfeatureid.h"
#include <QAbstractItemModel>

struct QgsSurveyObservation;
struct QgsSurveyObservationGroup;
struct QgsSurveyPoint;
struct QgsSurveyMeasurementDistance;
struct QgsSurveyMeasurementAngle;
class QgsSurveyLayer;

class CORE_EXPORT QgsSurveyObservationModel : public QAbstractItemModel
{
  public:
    explicit QgsSurveyObservationModel( QgsSurveyLayer *layer, QObject *parent = nullptr );

    QModelIndex indexForStation( const QgsFeatureId &id, const int column = 0 ) const;
    QModelIndex indexForObservationGroup( const QgsFeatureId &id, const int column = 0 ) const;
    QModelIndex indexForObservation( const QgsFeatureId &id, const int column = 0 ) const;
    QModelIndex indexFor( const QgsFeatureId &id, const int column = 0 ) const;
    QgsFeatureId idForIndex( const QModelIndex &index ) const;
    bool isStation( const QModelIndex &index ) const;
    bool isObservationGroup( const QModelIndex &index ) const;
    bool isObservation( const QModelIndex &index ) const;

    QModelIndex index( const int row, const int column, const QModelIndex &parent ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex & ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;

  private:
    QgsSurveyLayer *mLayer;

    QVector<QgsFeatureId> mStations;
    QHash<QgsFeatureId, QVector<QgsFeatureId>> mStationObservationGroups;

    QVariant data( const QgsSurveyPoint &point, const int column, const int role ) const;
    QVariant data( const QgsSurveyObservationGroup &group, const int column, const int role ) const;
    QVariant data( const QgsSurveyObservation &observation, const int column, const int role ) const;

    struct Item
    {
        QgsFeatureId id;
        enum Type
        {
          Station,
          ObservationGroup,
          Observation,
        } type;
    };

    QgsFeatureId mObservationGroup = FID_NULL;

    QVariant formatAngle( const QgsSurveyMeasurementAngle &a ) const;
    QVariant formatDistance( const QgsSurveyMeasurementDistance &d ) const;
};
