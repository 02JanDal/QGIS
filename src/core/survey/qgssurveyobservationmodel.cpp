#include "qgssurveyobservationmodel.h"

#include "qgssurveylayer.h"
#include "qgsunittypes.h"


QgsSurveyObservationModel::QgsSurveyObservationModel( QgsSurveyLayer *layer, QObject *parent )
  : QAbstractItemModel( parent ), mLayer( layer )
{
  for ( const QgsSurveyObservationGroup &og : mLayer->dataProvider()->observationGroups() )
  {
    if ( og.station == FID_NULL )
    {
      continue;
    }

    if ( !mStations.contains( og.station ) )
    {
      mStations.append( og.station );
      mStationObservationGroups.insert( og.station, {} );
    }
    mStationObservationGroups[og.station].append( og.id );
  }
}

QModelIndex QgsSurveyObservationModel::indexForStation( const QgsFeatureId &id, const int column ) const
{
  return index( mStations.indexOf( id ), column, QModelIndex() );
}
QModelIndex QgsSurveyObservationModel::indexForObservationGroup( const QgsFeatureId &id, const int column ) const
{
  const QgsSurveyObservationGroup og = mLayer->dataProvider()->observationGroup( id );
  return createIndex( mStationObservationGroups.value( og.station ).indexOf( id ), column, static_cast<quintptr>( id ) );
}
QModelIndex QgsSurveyObservationModel::indexForObservation( const QgsFeatureId &id, const int column ) const
{
  for ( const QgsSurveyObservationGroup &og : mLayer->dataProvider()->observationGroups() )
  {
    for ( int i = 0; i < og.observations.size(); ++i )
    {
      if ( og.observations.at( i ).id == id )
      {
        return createIndex( i, column, static_cast<quintptr>( id ) );
      }
    }
  }
  return QModelIndex();
}

QModelIndex QgsSurveyObservationModel::indexFor( const QgsFeatureId &id, const int column ) const
{
  return std::visit( [this, column, id]( auto &&arg ) {
    using T = std::decay_t<decltype( arg )>;
    if constexpr ( std::is_same_v<T, QgsSurveyFieldBookEntry> )
    {
      return QModelIndex();
    }
    else if ( std::is_same_v<T, QgsSurveyPoint> )
    {
      return indexForStation( id, column );
    }
    else if ( std::is_same_v<T, QgsSurveyObservationGroup> )
    {
      return indexForObservationGroup( id, column );
    }
    else if ( std::is_same_v<T, QgsSurveyObservation> )
    {
      return indexForObservation( id, column );
    }
  },
                     mLayer->dataProvider()->item( id ) );
}

bool QgsSurveyObservationModel::isStation( const QModelIndex &index ) const
{
  return std::holds_alternative<QgsSurveyPoint>( mLayer->dataProvider()->item( idForIndex( index ) ) );
}
bool QgsSurveyObservationModel::isObservationGroup( const QModelIndex &index ) const
{
  return std::holds_alternative<QgsSurveyObservationGroup>( mLayer->dataProvider()->item( idForIndex( index ) ) );
}
bool QgsSurveyObservationModel::isObservation( const QModelIndex &index ) const
{
  return std::holds_alternative<QgsSurveyObservation>( mLayer->dataProvider()->item( idForIndex( index ) ) );
}

QModelIndex QgsSurveyObservationModel::index( const int row, const int column, const QModelIndex &parent ) const
{
  if ( parent.isValid() )
  {
    return std::visit( [this, row, column]( auto &&arg ) {
      using T = std::decay_t<decltype( arg )>;
      if constexpr ( std::is_same_v<T, QgsSurveyFieldBookEntry> )
      {
        return QModelIndex();
      }
      else if constexpr ( std::is_same_v<T, QgsSurveyPoint> )
      {
        return createIndex( row, column, static_cast<quintptr>( mStationObservationGroups.value( arg.id ).at( row ) ) );
      }
      else if constexpr ( std::is_same_v<T, QgsSurveyObservationGroup> )
      {
        return createIndex( row, column, static_cast<quintptr>( arg.observations.at( row ).id ) );
      }
      else if constexpr ( std::is_same_v<T, QgsSurveyObservation> )
      {
        return QModelIndex();
      }
    },
                       mLayer->dataProvider()->item( idForIndex( parent ) ) );
  }
  else
  {
    return createIndex( row, column, static_cast<quintptr>( mStations.at( row ) ) );
  }
}

QModelIndex QgsSurveyObservationModel::parent( const QModelIndex &index ) const
{
  if ( index.isValid() )
  {
    return std::visit( [this]( auto &&arg ) {
      using T = std::decay_t<decltype( arg )>;
      if constexpr ( std::is_same_v<T, QgsSurveyFieldBookEntry> )
      {
        return QModelIndex();
      }
      else if constexpr ( std::is_same_v<T, QgsSurveyPoint> )
      {
        return QModelIndex();
      }
      else if constexpr ( std::is_same_v<T, QgsSurveyObservationGroup> )
      {
        return createIndex( mStations.indexOf( arg.station ), 0, static_cast<quintptr>( arg.station ) );
      }
      else if constexpr ( std::is_same_v<T, QgsSurveyObservation> )
      {
        for ( const QgsSurveyObservationGroup &og : mLayer->dataProvider()->observationGroups() )
        {
          for ( const QgsSurveyObservation &o : og.observations )
          {
            if ( o.id == arg.id )
            {
              return createIndex( mStationObservationGroups.value( og.station ).indexOf( og.id ), 0, static_cast<quintptr>( og.id ) );
            }
          }
        }
        return QModelIndex();
      }
    },
                       mLayer->dataProvider()->item( idForIndex( index ) ) );
  }
  else
  {
    return {};
  }
}

int QgsSurveyObservationModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
  {
    return std::visit( [this]( auto &&arg ) {
      using T = std::decay_t<decltype( arg )>;
      if constexpr ( std::is_same_v<T, QgsSurveyFieldBookEntry> )
      {
        return 0;
      }
      else if constexpr ( std::is_same_v<T, QgsSurveyPoint> )
      {
        return mStationObservationGroups.value( arg.id ).size();
      }
      else if constexpr ( std::is_same_v<T, QgsSurveyObservationGroup> )
      {
        return arg.observations.size();
      }
      else if constexpr ( std::is_same_v<T, QgsSurveyObservation> )
      {
        return 0;
      }
    },
                       mLayer->dataProvider()->item( idForIndex( parent ) ) );
  }
  else
  {
    return mStations.size();
  }
}

int QgsSurveyObservationModel::columnCount( const QModelIndex & ) const
{
  return 8;
}

QVariant QgsSurveyObservationModel::data( const QModelIndex &index, int role ) const
{
  Q_ASSERT( index.isValid() );

  return std::visit( [this, index, role]( auto &&arg ) {
    using T = std::decay_t<decltype( arg )>;
    if constexpr ( std::is_same_v<T, QgsSurveyFieldBookEntry> )
    {
      return QVariant();
    }
    else
    {
      return data( arg, index.column(), role );
    }
  },
                     mLayer->dataProvider()->item( idForIndex( index ) ) );
}

QVariant QgsSurveyObservationModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    switch ( section )
    {
      case 0:
        return tr( "Point" );
      case 1:
        return tr( "Type" );
      case 2:
        return tr( "Distance" );
      case 3:
        return tr( "Direction" );
      case 4:
        return tr( "Vertical angle" );
      case 5:
        return tr( "Reflector height" );
      case 6:
        return tr( "Attributes" );
      case 7:
        return tr( "Prism constant" );
    }
  }
  else if ( orientation == Qt::Horizontal && role == Qt::ToolTipRole )
  {
    switch ( section )
    {
      case 2:
        return tr( "Slope distance" );
      case 3:
        return tr( "Horizontal direction" );
      case 4:
        return tr( "Zenith angle" );
    }
  }
  return QVariant();
}

QgsFeatureId QgsSurveyObservationModel::idForIndex( const QModelIndex &index ) const
{
  Q_ASSERT( index.isValid() && index.model() == this );
  return static_cast<QgsFeatureId>( index.internalId() );
}

QVariant QgsSurveyObservationModel::data( const QgsSurveyPoint &point, const int column, const int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    switch ( column )
    {
      case 0:
        return tr( "Station: %1" ).arg( point.name );
      case 1:
        if ( point.type.has_value() )
        {
          switch ( point.type.value() )
          {
            case QgsSurveyPointType::New:
              return tr( "New" );
            case QgsSurveyPointType::Fix:
              return tr( "New" );
          }
        }
        else
        {
          return QVariant();
        }
      case 6:
        return point.attributes.keys().join( ", " );
    }
  }
  else if ( role == Qt::EditRole )
  {
    switch ( column )
    {
      case 0:
        return point.name;
      case 1:
        return point.type.has_value() ? QVariant::fromValue( point.type.value() ) : QVariant();
      case 6:
        return point.attributes;
    }
  }
  else if ( role == Qt::ToolTipRole )
  {
    switch ( column )
    {
      case 6:
      {
        QStringList rows;
        for ( auto it = point.attributes.cbegin(); it != point.attributes.cend(); ++it )
        {
          rows.append( QStringLiteral( "%1: %2" ).arg( it.key() ).arg( it.value().toString() ) );
        }
        return rows.join( "\n" );
      }
    }
  }
  return QVariant();
}

QVariant QgsSurveyObservationModel::data( const QgsSurveyObservationGroup &group, const int column, const int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    switch ( column )
    {
      case 0:
        return tr( "Group" );
      case 1:
        if ( group.aggregation.has_value() )
        {
          switch ( group.aggregation.value() )
          {
            case QgsSurveyObservationGroupAggregation::Rounds:
              return tr( "Rounds" );
            case QgsSurveyObservationGroupAggregation::Leveling_BF:
              return tr( "Leveling (BF)" );
            case QgsSurveyObservationGroupAggregation::Leveling_BFFB:
              return tr( "Leveling (BFFB)" );
            case QgsSurveyObservationGroupAggregation::Leveling_aBF:
              return tr( "Leveling (alternating BF)" );
            case QgsSurveyObservationGroupAggregation::Leveling_aBFFB:
              return tr( "Leveling (alternating BFFB)" );
          }
        }
        else
        {
          return QVariant();
        }
    }
  }
  else if ( role == Qt::EditRole )
  {
    switch ( column )
    {
      case 1:
        return group.aggregation.has_value() ? QVariant::fromValue( group.aggregation.value() ) : QVariant();
    }
  }
  return QVariant();
}

QVariant QgsSurveyObservationModel::data( const QgsSurveyObservation &observation, const int column, const int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    switch ( column )
    {
      case 0:
        return mLayer->dataProvider()->point( observation.point ).name;
      case 1:
        if ( observation.type.has_value() )
        {
          switch ( observation.type.value() )
          {
            case QgsSurveyObservationType::Point:
              return tr( "Point" );
            case QgsSurveyObservationType::BackSight:
              return tr( "Backsight" );
            case QgsSurveyObservationType::RoundMean:
              return tr( "Round mean" );
            case QgsSurveyObservationType::StationMean:
              return tr( "Station mean" );
          }
        }
        return QVariant();
      case 2:
        return observation.slopeDistance.has_value() ? formatDistance( observation.slopeDistance.value().toUnit( observation.slopeDistance->unit == Qgis::DistanceUnit::Millimeters ? Qgis::DistanceUnit::Meters : observation.slopeDistance->unit ) ) : QVariant();
      case 3:
        return observation.horizontalDirection.has_value() ? formatAngle( observation.horizontalDirection.value() ) : QVariant();
      case 4:
        return observation.zenithAngle.has_value() ? formatAngle( observation.zenithAngle.value() ) : QVariant();
      case 5:
        return observation.reflectorHeight.has_value() ? formatDistance( observation.reflectorHeight.value().toUnit( observation.reflectorHeight->unit == Qgis::DistanceUnit::Millimeters ? Qgis::DistanceUnit::Meters : observation.reflectorHeight->unit ) ) : QVariant();
      case 6:
        return observation.attributes.keys().join( ", " );
      case 7:
        return observation.prismConstant.has_value() ? QStringLiteral( "%1" ).arg( observation.prismConstant.value(), 0, 'f' ) : QVariant();
    }
  }
  else if ( role == Qt::EditRole )
  {
    switch ( column )
    {
      case 0:
        return observation.point;
      case 1:
        return observation.type.has_value() ? QVariant::fromValue( observation.type.value() ) : QVariant();
      case 2:
        return observation.slopeDistance.has_value() ? QVariant::fromValue( observation.slopeDistance->value ) : QVariant();
      case 3:
        return observation.horizontalDirection.has_value() ? QVariant::fromValue( observation.slopeDistance->value ) : QVariant();
      case 4:
        return observation.zenithAngle.has_value() ? QVariant::fromValue( observation.slopeDistance->value ) : QVariant();
      case 5:
        return observation.reflectorHeight.has_value() ? QVariant::fromValue( observation.slopeDistance->value ) : QVariant();
      case 6:
        return observation.attributes;
      case 7:
        return observation.prismConstant.has_value() ? QVariant::fromValue( observation.slopeDistance->value ) : QVariant();
    }
  }
  else if ( role == Qt::ToolTipRole )
  {
    switch ( column )
    {
      case 6:
      {
        QStringList rows;
        for ( auto it = observation.attributes.cbegin(); it != observation.attributes.cend(); ++it )
        {
          rows.append( QStringLiteral( "%1: %2" ).arg( it.key() ).arg( it.value().toString() ) );
        }
        return rows.join( "\n" );
      }
    }
  }
  else if ( role == Qt::TextAlignmentRole )
  {
    switch ( column )
    {
      case 2:
      case 3:
      case 4:
      case 5:
      case 7:
        return Qt::AlignRight;
    }
  }
  return QVariant();
}

QVariant QgsSurveyObservationModel::formatAngle( const QgsSurveyMeasurementAngle &a ) const
{
  switch ( a.unit )
  {
    case Qgis::AngleUnit::Gon:
      return QStringLiteral( "%1 gon" ).arg( a.value, 0, 'f', 4 );
    case Qgis::AngleUnit::Degrees:
      return QStringLiteral( "%1°" ).arg( a.value, 0, 'f', 6 );
    default:
      return formatAngle( a.toUnit( Qgis::AngleUnit::Gon ) );
  }
}

QVariant QgsSurveyObservationModel::formatDistance( const QgsSurveyMeasurementDistance &d ) const
{
  switch ( d.unit )
  {
    case Qgis::DistanceUnit::Millimeters:
      return QStringLiteral( "%1 %2" ).arg( d.value, 0, 'f', 2 ).arg( QgsUnitTypes::toAbbreviatedString( d.unit ) );
    case Qgis::DistanceUnit::Meters:
      return QStringLiteral( "%1 %2" ).arg( d.value, 0, 'f', 3 ).arg( QgsUnitTypes::toAbbreviatedString( d.unit ) );
    default:
      return QStringLiteral( "%1 %2" ).arg( d.value, 0, 'f' ).arg( QgsUnitTypes::toAbbreviatedString( d.unit ) );
  }
}
