#include "qgssdalconversion.h"
#include "qgssdalprovider.h"

#include <QFileInfo>
#include <QIcon>
#include <mutex>

#include <sdal/sdal.h>

#include "qgsproviderregistry.h"
#include "qgsapplication.h"
#include "qgsthreadingutils.h"

const QString SDAL_KEY = QStringLiteral( "sdal" );
const QString SDAL_DESCRIPTION = QObject::tr( "Survey data provider" );

QgsSdalProvider::QgsSdalProvider( const QString &uri, const ProviderOptions &providerOptions, Qgis::DataProviderReadFlags flags )
  : QgsSurveyDataProvider( uri, providerOptions, flags )
{
  try
  {
    const QVariantMap encodedUri = QgsProviderRegistry::instance()->decodeUri( SDAL_KEY, uri );
    sdal::drivers::OpenOptions options;
    for ( const QString &option : encodedUri.value( QStringLiteral( "openOptions" ), QStringList() ).toStringList() )
    {
      const QStringList parts = option.split( "=" );
      options.insert( { parts[0].toStdString(), parts.mid( 1 ).join( "=" ).toStdString() } );
    }
    mDataset = sdal::open( encodedUri.value( QStringLiteral( "path" ) ).toString().toStdString(), options );
    QgsSdalConversionContext context { {} };
    for ( const QgsSurveyObservationGroup &og : fromSdal<QgsSurveyObservationGroup>( mDataset->observations(), context ) )
    {
      mObservationGroups.insert( og.id, og );

      for ( const QgsSurveyObservation &o : og.observations )
      {
        mObservations.insert( o.id, o );
      }
    }
    for ( const QgsSurveyPoint &p : fromSdal<QgsSurveyPoint>( mDataset->points(), context ) )
    {
      mPoints.insert( p.id, p );
    }
    for ( const QgsSurveyFieldBookEntry &e : fromSdal<QgsSurveyFieldBookEntry>( mDataset->fieldBook(), context ) )
    {
      mFieldBook.insert( e.id, e );
    }
  }
  catch ( sdal::OpenError e )
  {
    setError( QgsError( QString::fromStdString( e.what() ), QStringLiteral( "SDAL Provider" ) ) );
  }
}

QgsSdalProvider::~QgsSdalProvider() = default;

QgsCoordinateReferenceSystem QgsSdalProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsCoordinateReferenceSystem();
}

bool QgsSdalProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return !!mDataset;
}

QString QgsSdalProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return SDAL_KEY;
}

QString QgsSdalProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return SDAL_DESCRIPTION;
}

QList<QgsSurveyFieldBookEntry> QgsSdalProvider::fieldBook() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  Q_ASSERT( isValid() );

  return mFieldBook.values();
}

QgsSurveyFieldBookEntry QgsSdalProvider::fieldBookEntry( const QgsFeatureId id ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mFieldBook.value( id );
}

int QgsSdalProvider::fieldBookCount() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mFieldBook.size();
}

QList<QgsSurveyPoint> QgsSdalProvider::points() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_ASSERT( isValid() );

  return pointsWithApproximations( mPoints.values() );
}

QgsSurveyPoint QgsSdalProvider::point( const QgsFeatureId id ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return pointWithApproximation( mPoints.value( id ) );
}

int QgsSdalProvider::pointCount() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mPoints.size();
}

QList<QgsSurveyObservationGroup> QgsSdalProvider::observationGroups() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_ASSERT( isValid() );

  return mObservationGroups.values();
}

QgsSurveyObservationGroup QgsSdalProvider::observationGroup( const QgsFeatureId id ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mObservationGroups.value( id );
}

int QgsSdalProvider::observationGroupsCount() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mObservationGroups.size();
}

QgsSurveyObservation QgsSdalProvider::observation( const QgsFeatureId id ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mObservations.value( id );
}

std::variant<QgsSurveyFieldBookEntry, QgsSurveyPoint, QgsSurveyObservationGroup, QgsSurveyObservation> QgsSdalProvider::item( const QgsFeatureId id ) const
{
  if ( mFieldBook.contains( id ) )
  {
    return mFieldBook.value( id );
  }
  else if ( mPoints.contains( id ) )
  {
    return mPoints.value( id );
  }
  else if ( mObservationGroups.contains( id ) )
  {
    return mObservationGroups.value( id );
  }
  else if ( mObservations.contains( id ) )
  {
    return mObservations.value( id );
  }
  else
  {
    return {};
  }
}

QgsSdalProvider *QgsSdalProvider::clone() const
{
  return new QgsSdalProvider( const_cast<QgsSdalProvider *>( this ) );
}

QgsSdalProvider::QgsSdalProvider( QgsSdalProvider *other )
  : QgsSurveyDataProvider( other ), mDataset( other->mDataset ) {}

QString QgsSdalProviderMetadata::sFilterString;
QStringList QgsSdalProviderMetadata::sExtensions;

QgsSdalProviderMetadata::QgsSdalProviderMetadata()
  : QgsProviderMetadata( SDAL_KEY, SDAL_DESCRIPTION )
{
}

QIcon QgsSdalProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconSurveyLayer.svg" ) );
}

QgsSdalProvider *QgsSdalProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  return new QgsSdalProvider( uri, options, flags );
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsSdalProviderMetadata::capabilities() const
{
  return ProviderMetadataCapability::LayerTypesForUri
         | ProviderMetadataCapability::PriorityForUri
         | ProviderMetadataCapability::QuerySublayers;
}

QVariantMap QgsSdalProviderMetadata::decodeUri( const QString &uri ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );
  QVariantMap uriComponents;
  if ( dsUri.param( QStringLiteral( "url" ) ).startsWith( "http" ) )
  {
    uriComponents.insert( QStringLiteral( "url" ), dsUri.param( QStringLiteral( "url" ) ) );
  }
  else
  {
    uriComponents.insert( QStringLiteral( "path" ), dsUri.param( QStringLiteral( "url" ) ) );
  }
  if ( dsUri.hasParam( "openOptions" ) )
  {
    uriComponents.insert( QStringLiteral( "openOptions" ), dsUri.params( QStringLiteral( "openOptions" ) ) );
  }
  return uriComponents;
}

QString QgsSdalProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri uri;
  uri.setParam( "url", parts.value( parts.contains( QStringLiteral( "path" ) ) ? QStringLiteral( "path" ) : QStringLiteral( "url" ) ).toString() );
  if ( parts.contains( "openOptions" ) )
  {
    uri.setParam( "openOptions", parts.value( QStringLiteral( "openOptions" ) ).toStringList() );
  }
  return uri.encodedUri();
}

int QgsSdalProviderMetadata::priorityForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  QString filePath = parts.value( QStringLiteral( "path" ) ).toString();
  const QFileInfo fi( filePath );

  if ( sExtensions.contains( fi.suffix(), Qt::CaseInsensitive ) )
    return 100;

  return 0;
}

QList<Qgis::LayerType> QgsSdalProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  QString filePath = parts.value( QStringLiteral( "path" ) ).toString();
  const QFileInfo fi( filePath );
  if ( sExtensions.contains( fi.suffix(), Qt::CaseInsensitive ) )
    return QList<Qgis::LayerType>() << Qgis::LayerType::PointCloud;

  return QList<Qgis::LayerType>();
}

QString QgsSdalProviderMetadata::filters( Qgis::FileFilterType type )
{
  switch ( type )
  {
    case Qgis::FileFilterType::Vector:
    case Qgis::FileFilterType::Raster:
    case Qgis::FileFilterType::Mesh:
    case Qgis::FileFilterType::MeshDataset:
    case Qgis::FileFilterType::VectorTile:
    case Qgis::FileFilterType::TiledScene:
    case Qgis::FileFilterType::PointCloud:
      return QString();

    case Qgis::FileFilterType::Survey:
      return sFilterString;
  }
  return QString();
}

QgsProviderMetadata::ProviderCapabilities QgsSdalProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QList<Qgis::LayerType> QgsSdalProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Survey };
}

void QgsSdalProviderMetadata::initProvider()
{
  sdal::initialize();
  buildSupportedSurveyFileFilterAndExtensions();
}

void QgsSdalProviderMetadata::buildSupportedSurveyFileFilterAndExtensions()
{
  static std::once_flag initialized;
  std::call_once( initialized, [] {
    QStringList filters;
    for ( const sdal::DriverMetadata &driver : sdal::driverMetadata() )
    {
      for ( const std::string &suffix : driver.fileNameSuffixes )
      {
        const QString extension = QString::fromStdString( suffix );
        sExtensions.append( extension );
        filters.append( QStringLiteral( "%1 (*%2 *%3)" ).arg( QString::fromStdString( driver.name ), extension, extension.toUpper() ) );
      }
    }
    filters.sort();
    sExtensions.sort();
    sFilterString = filters.join( ";;" );
  } );
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsSdalProviderMetadata();
}
