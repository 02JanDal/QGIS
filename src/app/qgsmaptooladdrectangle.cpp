/***************************************************************************
    qgsmaptooladdrectangle.h  -  map tool for adding rectangle
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooladdrectangle.h"
#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"

QgsMapToolAddRectangle::QgsMapToolAddRectangle( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), mode )
  , mParentTool( parentTool )
  , mTempRubberBand( nullptr )
  , mRectangle( QgsRectangle() )
{
  clean();
}

QgsMapToolAddRectangle::QgsMapToolAddRectangle( QgsMapCanvas *canvas )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget() )
  , mParentTool( nullptr )
  , mTempRubberBand( nullptr )
  , mRectangle( QgsRectangle() )
{
  clean();
}

QgsMapToolAddRectangle::~QgsMapToolAddRectangle()
{
  clean();
}

void QgsMapToolAddRectangle::keyPressEvent( QKeyEvent *e )
{
  if ( e && e->isAutoRepeat() )
  {
    return;
  }

  if ( e && e->key() == Qt::Key_Escape )
  {
    clean();
    if ( mParentTool )
      mParentTool->keyPressEvent( e );
  }
}

void QgsMapToolAddRectangle::keyReleaseEvent( QKeyEvent *e )
{
  if ( e && e->isAutoRepeat() )
  {
    return;
  }
}

QgsLineString *QgsMapToolAddRectangle::rectangleToLinestring( ) const
{
  std::unique_ptr<QgsLineString> ext( new QgsLineString() );
  if ( mRectangle.isEmpty() )
  {
    return ext.release();
  }

  ext->addVertex( QgsPoint( mRectangle.xMinimum(), mRectangle.yMinimum() ) );
  ext->addVertex( QgsPoint( mRectangle.xMinimum(), mRectangle.yMaximum() ) );
  ext->addVertex( QgsPoint( mRectangle.xMaximum(), mRectangle.yMaximum() ) );
  ext->addVertex( QgsPoint( mRectangle.xMaximum(), mRectangle.yMinimum() ) );
  ext->addVertex( QgsPoint( mRectangle.xMinimum(), mRectangle.yMinimum() ) );

  return ext.release();
}

QgsPolygonV2 *QgsMapToolAddRectangle::rectangleToPolygon() const
{
  std::unique_ptr<QgsPolygonV2> polygon( new QgsPolygonV2() );
  if ( mRectangle.isEmpty() )
  {
    return polygon.release();
  }

  polygon->setExteriorRing( rectangleToLinestring( ) );

  return polygon.release();
}

void QgsMapToolAddRectangle::deactivate()
{
  if ( !mParentTool || mRectangle.isEmpty() )
  {
    return;
  }

  mParentTool->clearCurve( );
  mParentTool->addCurve( rectangleToLinestring() );
  delete mTempRubberBand;
  mTempRubberBand = nullptr;
  mPoints.clear();

  QgsMapToolCapture::deactivate();
}

void QgsMapToolAddRectangle::activate()
{
  clean();
  QgsMapToolCapture::activate();
}

void QgsMapToolAddRectangle::clean()
{
  if ( mTempRubberBand )
  {
    delete mTempRubberBand;
    mTempRubberBand = nullptr;
  }
  mPoints.clear();
  if ( mParentTool )
  {
    mParentTool->deleteTempRubberBand();
  }
}
