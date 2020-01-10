/***************************************************************************
  qgsabstractvectorlayer3drenderer.cpp
  --------------------------------------
  Date                 : January 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsabstractvectorlayer3drenderer.h"

#include "qgsvectorlayer.h"


QgsAbstractVectorLayer3DRenderer::QgsAbstractVectorLayer3DRenderer()
{
}

void QgsAbstractVectorLayer3DRenderer::setLayer( QgsVectorLayer *layer )
{
  mLayerRef = QgsMapLayerRef( layer );
}

QgsVectorLayer *QgsAbstractVectorLayer3DRenderer::layer() const
{
  return qobject_cast<QgsVectorLayer *>( mLayerRef.layer );
}

void QgsAbstractVectorLayer3DRenderer::copyBaseProperties( QgsAbstractVectorLayer3DRenderer *r ) const
{
  r->mLayerRef = mLayerRef;
}

void QgsAbstractVectorLayer3DRenderer::writeXmlBaseProperties( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )
  elem.setAttribute( QStringLiteral( "layer" ), mLayerRef.layerId );
}

void QgsAbstractVectorLayer3DRenderer::readXmlBaseProperties( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  mLayerRef = QgsMapLayerRef( elem.attribute( QStringLiteral( "layer" ) ) );
}

void QgsAbstractVectorLayer3DRenderer::resolveReferences( const QgsProject &project )
{
  mLayerRef.setLayer( project.mapLayer( mLayerRef.layerId ) );
}
