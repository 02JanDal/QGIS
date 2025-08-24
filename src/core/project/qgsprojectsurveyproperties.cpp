#include "qgscoordinatereferencesystem.h"
#include "qgsprojectsurveyproperties.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QCoreApplication>
#include <QDomElement>
#include <QThread>

QgsProjectSurveyProperties::QgsProjectSurveyProperties( QObject *parent )
  : QObject( parent ), mKnownPointsProvider( std::make_unique<QgsNoKnownPointsProvider>() ) {}

void QgsProjectSurveyProperties::reset()
{
  mKnownPointsProvider = std::make_unique<QgsNoKnownPointsProvider>();
  emit changed();
}
void QgsProjectSurveyProperties::resolveReferences( const QgsProject *project )
{
  if ( mKnownPointsProvider )
  {
    mKnownPointsProvider->resolveReferences( project );
  }
}

bool QgsProjectSurveyProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement providerElement = element.firstChildElement( QStringLiteral( "KnownPointsProvider" ) );
  if ( !providerElement.isNull() )
  {
    const QString type = providerElement.attribute( QStringLiteral( "type" ) );
    if ( type == QLatin1String( "no" ) )
      mKnownPointsProvider = std::make_unique< QgsNoKnownPointsProvider >();
    else if ( type == QLatin1String( "vector" ) )
      mKnownPointsProvider = std::make_unique< QgsVectorKnownPointsProvider >();
    else
      mKnownPointsProvider = std::make_unique< QgsNoKnownPointsProvider >();

    mKnownPointsProvider->readXml( providerElement, context );
  }
  else
  {
    mKnownPointsProvider = std::make_unique< QgsNoKnownPointsProvider >();
  }

  emit changed();
  return true;
}
QDomElement QgsProjectSurveyProperties::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( QStringLiteral( "SurveyProperties" ) );

  if ( mKnownPointsProvider )
  {
    QDomElement providerElement = mKnownPointsProvider->writeXml( document, context );
    providerElement.setAttribute( QStringLiteral( "type" ), mKnownPointsProvider->type() );
    element.appendChild( providerElement );
  }

  return element;
}
void QgsProjectSurveyProperties::setKnownPointsProvider( QgsAbstractKnownPointsProvider *provider )
{
  if ( mKnownPointsProvider.get() == provider )
    return;

  const bool hasChanged = ( provider && mKnownPointsProvider ) ? !mKnownPointsProvider->equals( provider ) : ( static_cast< bool >( provider ) != static_cast< bool >( mKnownPointsProvider.get() ) );

  mKnownPointsProvider.reset( provider );
  if ( hasChanged )
    emit changed();
}


QgsAbstractKnownPointsProvider::QgsAbstractKnownPointsProvider( const QgsAbstractKnownPointsProvider & ) {}
QgsAbstractKnownPointsProvider::~QgsAbstractKnownPointsProvider() = default;
void QgsAbstractKnownPointsProvider::resolveReferences( const QgsProject * ) {}
void QgsAbstractKnownPointsProvider::writeCommonProperties( QDomElement &, const QgsReadWriteContext & ) const {}
void QgsAbstractKnownPointsProvider::readCommonProperties( const QDomElement &, const QgsReadWriteContext & ) {}


bool QgsNoKnownPointsProvider::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  readCommonProperties( element, context );
  return true;
}
QDomElement QgsNoKnownPointsProvider::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( QStringLiteral( "KnownPointsProvider" ) );
  writeCommonProperties( element, context );
  return element;
}
QgsCoordinateReferenceSystem QgsNoKnownPointsProvider::crs() const
{
  return QgsCoordinateReferenceSystem();
}
QgsNoKnownPointsProvider *QgsNoKnownPointsProvider::clone() const
{
  return new QgsNoKnownPointsProvider( *this );
}
void QgsNoKnownPointsProvider::prepare()
{
  Q_ASSERT_X( QThread::currentThread() == QCoreApplication::instance()->thread(), "QgsNoKnownPointsProvider::prepare", "prepare() must be called from the main thread" );
}
bool QgsNoKnownPointsProvider::equals( const QgsAbstractKnownPointsProvider *other ) const
{
  return other->type() == type();
}


QgsVectorKnownPointsProvider::QgsVectorKnownPointsProvider( const QgsVectorKnownPointsProvider &other )
  : QgsAbstractKnownPointsProvider( other ), mKeyAttribute( other.mKeyAttribute ), mVectorLayer( other.mVectorLayer ) {}
void QgsVectorKnownPointsProvider::resolveReferences( const QgsProject *project )
{
  if ( mVectorLayer )
    return;

  mVectorLayer.resolve( project );
}
bool QgsVectorKnownPointsProvider::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QString layerId = element.attribute( QStringLiteral( "layer" ) );
  const QString layerName = element.attribute( QStringLiteral( "layerName" ) );
  const QString layerSource = element.attribute( QStringLiteral( "layerSource" ) );
  const QString layerProvider = element.attribute( QStringLiteral( "layerProvider" ) );
  mVectorLayer = _LayerRef<QgsVectorLayer>( layerId, layerName, layerSource, layerProvider );

  mKeyAttribute = element.attribute( QStringLiteral( "keyAttribute" ) );

  readCommonProperties( element, context );
  return true;
}
QDomElement QgsVectorKnownPointsProvider::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( QStringLiteral( "KnownPointsProvider" ) );
  if ( mVectorLayer )
  {
    element.setAttribute( QStringLiteral( "layer" ), mVectorLayer.layerId );
    element.setAttribute( QStringLiteral( "layerName" ), mVectorLayer.name );
    element.setAttribute( QStringLiteral( "layerSource" ), mVectorLayer.source );
    element.setAttribute( QStringLiteral( "layerProvider" ), mVectorLayer.provider );
  }

  element.setAttribute( QStringLiteral( "keyAttribute" ), mKeyAttribute );

  writeCommonProperties( element, context );
  return element;
}
QgsCoordinateReferenceSystem QgsVectorKnownPointsProvider::crs() const
{
  return mClonedVectorLayer ? mClonedVectorLayer->crs()
                            : ( mVectorLayer ? mVectorLayer->crs() : QgsCoordinateReferenceSystem() );
}
QgsVectorKnownPointsProvider *QgsVectorKnownPointsProvider::clone() const
{
  return new QgsVectorKnownPointsProvider( *this );
}
void QgsVectorKnownPointsProvider::prepare()
{
  Q_ASSERT_X( QThread::currentThread() == QCoreApplication::instance()->thread(), "QgsNoKnownPointsProvider::prepare", "prepare() must be called from the main thread" );

  if ( mVectorLayer && mVectorLayer->isValid() )
  {
    mClonedVectorLayer.reset( mVectorLayer->clone() );
  }
}
bool QgsVectorKnownPointsProvider::equals( const QgsAbstractKnownPointsProvider *other ) const
{
  if ( other->type() != type() )
    return false;

  const QgsVectorKnownPointsProvider *otherPoints = qgis::down_cast< const QgsVectorKnownPointsProvider * >( other );
  if ( mVectorLayer.get() != otherPoints->layer() || mKeyAttribute != otherPoints->mKeyAttribute )
    return false;

  return true;
}
void QgsVectorKnownPointsProvider::setLayer( QgsVectorLayer *layer )
{
  if ( QgsWkbTypes::flatType( layer->wkbType() ) != Qgis::WkbType::Point )
  {
    QgsDebugMsgLevel( QStringLiteral( "Unable to set vector layer as it has wrong geometry type" ), 2 );
  }
  else
  {
    mVectorLayer.setLayer( layer );
  }
}
QgsVectorLayer *QgsVectorKnownPointsProvider::layer() const
{
  return mVectorLayer.get();
}

QHash<QString, QgsPoint> QgsVectorKnownPointsProvider::points( const QSet<QString> & ) const
{
  // TODO SURVEY filter by given IDs on mKeyAttribute
  QgsVectorLayer *layer = mClonedVectorLayer.get() ? mClonedVectorLayer.get() : mVectorLayer.get();
  QHash<QString, QgsPoint> result;
  QgsFeature f;
  QgsFeatureIterator it = layer->dataProvider()->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QSet<QString> { mKeyAttribute }, layer->fields() ) );
  while ( it.nextFeature( f ) )
  {
    result.insert( f.attribute( mKeyAttribute ).toString(), *qgsgeometry_cast<const QgsPoint *>( f.geometry().constGet() ) );
  }
  return result;
}
