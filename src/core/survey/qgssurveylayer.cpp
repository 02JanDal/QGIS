#include "qgslayermetadataformatter.h"
#include "qgsruntimeprofiler.h"
#include "qgssurveylayer.h"
#include "qgssurveylayerrenderer.h"
#include "qgsthreadingutils.h"

#include <qgsapplication.h>
#include <qgsmaplayerfactory.h>
#include <qgsproviderregistry.h>
#include <qgsxmlutils.h>

QgsSurveyLayer::QgsSurveyLayer( const QString &uri, const QString &baseName, const QString &providerLib, const LayerOptions &options )
  : QgsMapLayer( Qgis::LayerType::Survey, uri, baseName ), mLayerOptions( options )
{
  if ( !uri.isEmpty() && !providerLib.isEmpty() )
  {
    const QgsDataProvider::ProviderOptions providerOptions { options.transformContext };
    Qgis::DataProviderReadFlags providerFlags;
    setDataSource( uri, baseName, providerLib, providerOptions, providerFlags );
  }
}

void QgsSurveyLayer::select( QgsFeatureId featureId )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mSelectedFeatureIds.insert( featureId );

  emit selectionChanged( QgsFeatureIds() << featureId, QgsFeatureIds(), false );
}

void QgsSurveyLayer::select( const QgsFeatureIds &featureIds )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mSelectedFeatureIds.unite( featureIds );

  emit selectionChanged( featureIds, QgsFeatureIds(), false );
}

void QgsSurveyLayer::deselect( QgsFeatureId featureId )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mSelectedFeatureIds.remove( featureId );

  emit selectionChanged( QgsFeatureIds(), QgsFeatureIds() << featureId, false );
}

void QgsSurveyLayer::deselect( const QgsFeatureIds &featureIds )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mSelectedFeatureIds.subtract( featureIds );

  emit selectionChanged( QgsFeatureIds(), featureIds, false );
}

void QgsSurveyLayer::removeSelection()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mSelectedFeatureIds.isEmpty() )
    return;

  selectByIds( QgsFeatureIds() );
}


int QgsSurveyLayer::selectedFeatureCount() const
{
  return mSelectedFeatureIds.size();
}

void QgsSurveyLayer::selectByIds( const QgsFeatureIds &ids, Qgis::SelectBehavior behavior )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsFeatureIds newSelection;

  switch ( behavior )
  {
    case Qgis::SelectBehavior::SetSelection:
      newSelection = ids;
      break;

    case Qgis::SelectBehavior::AddToSelection:
      newSelection = mSelectedFeatureIds + ids;
      break;

    case Qgis::SelectBehavior::RemoveFromSelection:
      newSelection = mSelectedFeatureIds - ids;
      break;

    case Qgis::SelectBehavior::IntersectSelection:
      newSelection = mSelectedFeatureIds.intersect( ids );
      break;
  }

  QgsFeatureIds deselectedFeatures = mSelectedFeatureIds - newSelection;
  mSelectedFeatureIds = newSelection;

  emit selectionChanged( newSelection, deselectedFeatures, true );
}

void QgsSurveyLayer::modifySelection( const QgsFeatureIds &selectIds, const QgsFeatureIds &deselectIds )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsFeatureIds intersectingIds = selectIds & deselectIds;
  if ( !intersectingIds.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Trying to select and deselect the same item at the same time. Unsure what to do. Selecting dubious items." ), 3 );
  }

  QgsFeatureIds selectedIds = selectIds - mSelectedFeatureIds;
  QgsFeatureIds deselectedIds = mSelectedFeatureIds & deselectIds;
  if ( selectedIds.isEmpty() && deselectedIds.isEmpty() )
  {
    return;
  }

  mSelectedFeatureIds -= deselectedIds;
  mSelectedFeatureIds += selectedIds;

  emit selectionChanged( selectedIds, deselectedIds - intersectingIds, false );
}

void QgsSurveyLayer::invertSelection()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsFeatureIds ids = allFeatureIds();
  ids.subtract( mSelectedFeatureIds );
  selectByIds( ids );
}

void QgsSurveyLayer::selectAll()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  selectByIds( allFeatureIds() );
}

QgsFeatureIds QgsSurveyLayer::allFeatureIds() const
{
  QgsFeatureIds ids;
  for ( const QgsSurveyPoint &p : mDataProvider->points() )
  {
    ids.insert( p.id );
  }
  for ( const QgsSurveyFieldBookEntry &e : mDataProvider->fieldBook() )
  {
    ids.insert( e.id );
  }
  for ( const QgsSurveyObservationGroup &og : mDataProvider->observationGroups() )
  {
    ids.insert( og.id );

    for ( const QgsSurveyObservation &o : og.observations )
    {
      ids.insert( o.id );
    }
  }
  return ids;
}


QgsMapLayer *QgsSurveyLayer::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsSurveyLayer *layer = new QgsSurveyLayer( source(), name(), mProviderKey, mLayerOptions );
  QgsMapLayer::clone( layer );
  layer->mLayerOptions = mLayerOptions;

  return layer;
}

QgsSurveyDataProvider *QgsSurveyLayer::dataProvider()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider.get();
}

const QgsSurveyDataProvider *QgsSurveyLayer::dataProvider() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider.get();
}

QgsMapLayerRenderer *QgsSurveyLayer::createMapRenderer(QgsRenderContext &rendererContext)
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return new QgsSurveyLayerRenderer( this, rendererContext );
}

QgsRectangle QgsSurveyLayer::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mDataProvider )
    return QgsRectangle();

  return mDataProvider->extent();
}

QString QgsSurveyLayer::loadDefaultMetadata(bool &resultFlag)
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  resultFlag = false;
  if ( !mDataProvider || !mDataProvider->isValid() )
    return QString();

  if ( qgis::down_cast< QgsSurveyDataProvider * >( mDataProvider.get() )->capabilities() & Qgis::SurveyProviderCapability::ReadLayerMetadata )
  {
    setMetadata( mDataProvider->layerMetadata() );
  }
  else
  {
    QgsMapLayer::loadDefaultMetadata( resultFlag );
  }
  resultFlag = true;
  return QString();
}

bool QgsSurveyLayer::readSymbology(const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, StyleCategories categories)
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QDomElement elem = node.toElement();

  readCommonStyle( elem, context, categories );

  readStyle( node, errorMessage, context, categories );

  if ( categories.testFlag( CustomProperties ) )
    readCustomProperties( node, QStringLiteral( "variable" ) );

  return true;
}

bool QgsSurveyLayer::readStyle(const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, StyleCategories categories)
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  bool result = true;

  return result;
}

bool QgsSurveyLayer::writeSymbology(QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context, StyleCategories categories) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( errorMessage )

  QDomElement elem = node.toElement();
  writeCommonStyle( elem, doc, context, categories );

  ( void ) writeStyle( node, doc, errorMessage, context, categories );

  return true;
}

bool QgsSurveyLayer::writeStyle(QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context, StyleCategories categories) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return true;
}

QString QgsSurveyLayer::htmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QgsLayerMetadataFormatter htmlFormatter( metadata() );
  QString myMetadata = QStringLiteral( "<html>\n<body>\n" );

  myMetadata += generalHtmlMetadata();

  // Begin Provider section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Information from provider" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += QLatin1String( "<table class=\"list-view\">\n" );

  if ( mDataProvider )
    myMetadata += mDataProvider->htmlMetadata();

  // Extent
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Extent" ) + QStringLiteral( "</td><td>" ) + extent().toString() + QStringLiteral( "</td></tr>\n" );

  myMetadata += QLatin1String( "</table>\n<br><br>" );

  // CRS
  myMetadata += crsHtmlMetadata();

  // identification section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Identification" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.identificationSectionHtml();
  myMetadata += QLatin1String( "<br><br>\n" );

  // extent section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Extent" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.extentSectionHtml( isSpatial() );
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the Access section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Access" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.accessSectionHtml();
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the contacts section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Contacts" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.contactsSectionHtml();
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the links section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Links" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.linksSectionHtml();
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the history section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "History" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.historySectionHtml();
  myMetadata += QLatin1String( "<br><br>\n" );

  myMetadata += customPropertyHtmlMetadata();

  myMetadata += QLatin1String( "\n</body>\n</html>\n" );
  return myMetadata;
}

void QgsSurveyLayer::reload()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mDataProvider )
  {
    mDataProvider->reloadData();
  }
}

void QgsSurveyLayer::setTransformContext(const QgsCoordinateTransformContext &transformContext)
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mDataProvider )
    mDataProvider->setTransformContext( transformContext );
  invalidateWgs84Extent();
}

bool QgsSurveyLayer::readXml( const QDomNode &layerNode, QgsReadWriteContext &context )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // create provider
  const QDomNode pkeyNode = layerNode.namedItem( QStringLiteral( "provider" ) );
  mProviderKey = pkeyNode.toElement().text();

  if ( !( mReadFlags & QgsMapLayer::FlagDontResolveLayers ) )
  {
    const QgsDataProvider::ProviderOptions providerOptions { context.transformContext() };
    Qgis::DataProviderReadFlags flags = providerReadFlags( layerNode, mReadFlags );
    // read extent
    if ( mReadFlags & QgsMapLayer::FlagReadExtentFromXml )
    {
      const QDomNode extentNode = layerNode.namedItem( QStringLiteral( "extent" ) );
      if ( !extentNode.isNull() )
      {
        // get the extent
        const QgsRectangle mbr = QgsXmlUtils::readRectangle( extentNode.toElement() );

        // store the extent
        setExtent( mbr );
      }
    }

    setDataSource( mDataSource, mLayerName, mProviderKey, providerOptions, flags );
  }

  QString errorMsg;
  if ( !readSymbology( layerNode, errorMsg, context ) )
    return false;

  readStyleManager( layerNode );

  return isValid();
}

bool QgsSurveyLayer::writeXml( QDomNode &layerNode, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QDomElement mapLayerNode = layerNode.toElement();
  mapLayerNode.setAttribute( QStringLiteral( "type" ), QgsMapLayerFactory::typeToString( Qgis::LayerType::Survey ) );

  {
    QDomElement provider = doc.createElement( QStringLiteral( "provider" ) );
    const QDomText providerText = doc.createTextNode( providerType() );
    provider.appendChild( providerText );
    layerNode.appendChild( provider );
  }

  writeStyleManager( layerNode, doc );

  QString errorMsg;
  return writeSymbology( layerNode, doc, errorMsg, context );
}

void QgsSurveyLayer::setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  setName( baseName );
  mProviderKey = provider;
  mDataSource = dataSource;

  if ( mPreloadedProvider )
  {
    mDataProvider.reset( qobject_cast< QgsSurveyDataProvider * >( mPreloadedProvider.release() ) );
  }
  else
  {
    std::unique_ptr< QgsScopedRuntimeProfile > profile;
    if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
      profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Create %1 provider" ).arg( provider ), QStringLiteral( "projectload" ) );
    mDataProvider.reset( qobject_cast<QgsSurveyDataProvider *>( QgsProviderRegistry::instance()->createProvider( provider, dataSource, options, flags ) ) );
  }

  if ( !mDataProvider )
  {
    QgsDebugError( QStringLiteral( "Unable to get survey data provider" ) );
    setValid( false );
    return;
  }

  mDataProvider->setParent( this );
  QgsDebugMsgLevel( QStringLiteral( "Instantiated the survey data provider plugin" ), 2 );

  setValid( mDataProvider->isValid() );
  if ( !isValid() )
  {
    QgsDebugError( QStringLiteral( "Invalid survey data provider plugin %1" ).arg( QString( mDataSource.toUtf8() ) ) );
    setError( mDataProvider->error() );
    return;
  }

  // Load initial extent, crs and renderer
  setCrs( mDataProvider->crs() );
  if ( !( flags & Qgis::DataProviderReadFlag::SkipGetExtent ) )
  {
    setExtent( mDataProvider->extent() );
  }

  connect( mDataProvider.get(), &QgsSurveyDataProvider::approximationStateChanged, this, [this]() {
    if ( mDataProvider->approximationState() != QgsSurveyDataProvider::Approximated )
    {
      emit recalculateExtents();
      triggerRepaint();
    }
  } );

  // start approximating later (necessary when loading a project, as otherwise the known points provider may not have been loaded yet)
  QMetaObject::invokeMethod( mDataProvider.get(), &QgsSurveyDataProvider::startApproximating, Qt::QueuedConnection );
}
