#include "qgsfeedback.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsproject.h"
#include "qgsprojectsurveyproperties.h"
#include "qgsrendercontext.h"
#include "qgssurveylayerrenderer.h"

QgsSurveyLayerRenderer::QgsSurveyLayerRenderer( QgsSurveyLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context ), mDataProvider( layer->dataProvider()->clone() ), mFeedback( new QgsFeedback )
{
  {
    QgsSimpleMarkerSymbolLayer *layer = new QgsSimpleMarkerSymbolLayer( Qgis::MarkerShape::Triangle );
    layer->setColor( QStringLiteral( "#ff0000" ) );
    layer->setStrokeColor( QStringLiteral( "#aa0000" ) );
    layer->setStrokeWidth( 2 );

    mSymbolFix = std::make_unique<QgsMarkerSymbol>( QgsSymbolLayerList { layer } );
  }
  {
    QgsSimpleMarkerSymbolLayer *layer = new QgsSimpleMarkerSymbolLayer( Qgis::MarkerShape::Triangle );
    layer->setColor( QStringLiteral( "#0000ff" ) );
    layer->setStrokeColor( QStringLiteral( "#0000aa" ) );
    layer->setStrokeWidth( 2 );

    mSymbolStationFix = std::make_unique<QgsMarkerSymbol>( QgsSymbolLayerList { layer } );
  }
  {
    QgsSimpleMarkerSymbolLayer *layer = new QgsSimpleMarkerSymbolLayer();
    layer->setColor( QStringLiteral( "#0000ff" ) );
    layer->setStrokeColor( QStringLiteral( "#0000aa" ) );
    layer->setStrokeWidth( 2 );

    mSymbolStation = std::make_unique<QgsMarkerSymbol>( QgsSymbolLayerList { layer } );
  }
  {
    QgsSimpleMarkerSymbolLayer *layer = new QgsSimpleMarkerSymbolLayer();
    layer->setColor( QStringLiteral( "#ff0000" ) );
    layer->setStrokeColor( QStringLiteral( "#aa0000" ) );
    layer->setStrokeWidth( 2 );

    mSymbolPoint = std::make_unique<QgsMarkerSymbol>( QgsSymbolLayerList { layer } );
  }
}

QgsSurveyLayerRenderer::~QgsSurveyLayerRenderer() = default;

bool QgsSurveyLayerRenderer::render()
{
  if ( !mDataProvider )
  {
    return false;
  }

  QgsRenderContext &context = *renderContext();
  QPainter *painter = context.painter();
  const QgsScopedQPainterState painterState( painter );

  mSymbolFix->startRender( context );
  mSymbolStationFix->startRender( context );
  mSymbolStation->startRender( context );
  mSymbolPoint->startRender( context );

  const QList<QgsSurveyPoint> points = mDataProvider->points();
  const QList<QgsSurveyObservationGroup> observations = mDataProvider->observationGroups();
  const QSet<QString> knownPoints = QgsProject::instance()->surveyProperties()->knownPointsProvider()->points( {} ).keys().toSet();

  QSet<QPair<QgsFeatureId, QgsFeatureId>> betweenPoints;
  for ( const QgsSurveyObservationGroup &og : observations )
  {
    for ( const QgsSurveyObservation &o : og.observations )
    {
      if ( o.station != FID_NULL )
      {
        betweenPoints.insert( o.station < o.point ? qMakePair( o.station, o.point ) : qMakePair( o.point, o.station ) );
      }
    }
  }
  for ( const QPair<QgsFeatureId, QgsFeatureId> &between : betweenPoints )
  {
    QgsSurveyPoint p1, p2;
    for ( const QgsSurveyPoint &p : points )
    {
      if ( p.id == between.first )
      {
        p1 = p;
      }
      if ( p.id == between.second )
      {
        p2 = p;
      }
    }

    if ( p1.coordinates.isEmpty() || p2.coordinates.isEmpty() )
    {
      continue;
    }
    const QgsSurveyCoordinate c1 = p1.bestCoordinate().second;
    const QgsSurveyCoordinate c2 = p2.bestCoordinate().second;
    if ( !c1.xy.has_value() || !c2.xy.has_value() )
    {
      continue;
    }

    painter->setPen( QPen( Qt::black, 2 ) );
    painter->drawLine( context.mapToPixel().transform( c1.xy->toPointXY() ).toQPointF(), context.mapToPixel().transform( c2.xy->toPointXY() ).toQPointF() );
  }

  for ( const QgsSurveyPoint &point : points )
  {
    if ( point.coordinates.isEmpty() )
    {
      continue;
    }
    const QgsSurveyCoordinate coord = point.bestCoordinate().second;
    if ( !coord.xy.has_value() )
    {
      continue;
    }

    const bool isFix = point.type == QgsSurveyPointType::Fix || knownPoints.contains( point.name );
    const QgsPointXY xy = renderContext()->mapToPixel().transform( coord.xy->toPointXY() );

    if ( isFix )
    {
      mSymbolFix->renderPoint( xy.toQPointF(), nullptr, context );
    }
    else
    {
      mSymbolPoint->renderPoint( xy.toQPointF(), nullptr, context );
    }
  }

  mSymbolFix->stopRender( context );
  mSymbolStationFix->stopRender( context );
  mSymbolStation->stopRender( context );
  mSymbolPoint->stopRender( context );

  return true;
}

Qgis::MapLayerRendererFlags QgsSurveyLayerRenderer::flags() const
{
  return Qgis::MapLayerRendererFlags();
}
