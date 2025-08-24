/***************************************************************************
  qgssurveydataprovider.cpp - QgsSurveyDataProvider

 ---------------------
 begin                : 23.7.2025
 copyright            : (C) 2025 by jan
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgssurveydataprovider.h"
#include "qgsthreadingutils.h"

QgsSurveyDataProvider::QgsSurveyDataProvider(const QString& uri, const ProviderOptions& providerOptions, Qgis::DataProviderReadFlags flags): QgsDataProvider( uri, providerOptions, flags )
{
}

Qgis::SurveyProviderCapabilities QgsSurveyDataProvider::capabilities() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return Qgis::SurveyProviderCapabilities();
}

QgsSurveyDataProvider::SurveyCoordinateApproximationState QgsSurveyDataProvider::approximationState() const
{
  if ( mApproximationFinished )
  {
    return Approximated;
  }
  else if ( mApproximatingTask )
  {
    return Approximating;
  }
  else
  {
    return NotApproximated;
  }
}

void QgsSurveyDataProvider::startApproximating()
{
  mApproximatingTask = new QgsSurveyCoordinateApproximationTask( this );
  connect( mApproximatingTask, &QgsSurveyCoordinateApproximationTask::taskCompleted, this, [this]() {
    mApproximatedCoordinates = mApproximatingTask->result();
    mApproximationFinished = true;
    mApproximatingTask = nullptr;
    emit approximationStateChanged();
  } );
  connect( mApproximatingTask, &QgsSurveyCoordinateApproximationTask::taskTerminated, this, [this]() {
    mApproximationFinished = true;
    mApproximatingTask = nullptr;
    emit approximationStateChanged();
  } );
  QgsApplication::taskManager()->addTask( mApproximatingTask );
  emit approximationStateChanged();
}

QList<QgsSurveyPoint> QgsSurveyDataProvider::pointsWithApproximations( const QList<QgsSurveyPoint> &points ) const
{
  if ( mApproximatedCoordinates.isEmpty() )
  {
    return points;
  }

  QList<QgsSurveyPoint> output = points;
  for ( QgsSurveyPoint &point : output )
  {
    if ( !point.coordinates.contains( QgsSurveyCoordinateSource::Approximated ) && mApproximatedCoordinates.contains( point.name ) )
    {
      point.coordinates.insert( QgsSurveyCoordinateSource::Approximated, mApproximatedCoordinates.value( point.name ) );
    }
  }
  return output;
}

QgsSurveyPoint QgsSurveyDataProvider::pointWithApproximation( const QgsSurveyPoint &point ) const
{
  if ( mApproximatedCoordinates.isEmpty() )
  {
    return point;
  }

  QgsSurveyPoint output = point;
  if ( !output.coordinates.contains( QgsSurveyCoordinateSource::Approximated ) && mApproximatedCoordinates.contains( output.name ) )
  {
    output.coordinates.insert( QgsSurveyCoordinateSource::Approximated, mApproximatedCoordinates.value( output.name ) );
  }
  return output;
}

QgsSurveyDataProvider::QgsSurveyDataProvider( QgsSurveyDataProvider *other )
  : mApproximatingTask( other->mApproximatingTask ), mApproximatedCoordinates( other->mApproximatedCoordinates ), mApproximationFinished( other->mApproximationFinished )
{
  if ( mApproximatingTask )
  {
    connect( mApproximatingTask, &QgsSurveyCoordinateApproximationTask::taskCompleted, this, [this]() {
      mApproximatedCoordinates = mApproximatingTask->result();
      mApproximationFinished = true;
      mApproximatingTask = nullptr;
      emit approximationStateChanged();
    } );
    connect( mApproximatingTask, &QgsSurveyCoordinateApproximationTask::taskTerminated, this, [this]() {
      mApproximationFinished = true;
      mApproximatingTask = nullptr;
      emit approximationStateChanged();
    } );
  }
}

QgsRectangle QgsSurveyDataProvider::extent() const
{
  QgsRectangle rect;
  for ( const QgsSurveyPoint &point : points() )
  {
    if ( point.coordinates.isEmpty() )
    {
      continue;
    }
    const QgsSurveyCoordinate coord = point.bestCoordinate().second;
    if ( coord.xy.has_value() )
    {
      rect.include( QgsPointXY( coord.xy->x, coord.xy->y ) );
    }
  }
  return rect;
}
