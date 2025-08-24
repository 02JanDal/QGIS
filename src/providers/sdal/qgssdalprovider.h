#pragma once

#include "qgsfeatureid.h"
#include "qgsprovidermetadata.h"
#include "qgssurveydataprovider.h"

namespace sdal
{
  class Dataset;
}

class QgsSdalProvider : public QgsSurveyDataProvider
{
    Q_OBJECT
  public:
    QgsSdalProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() );
    virtual ~QgsSdalProvider();

    QgsCoordinateReferenceSystem crs() const override;
    bool isValid() const override;
    QString name() const override;
    QString description() const override;

    QList<QgsSurveyFieldBookEntry> fieldBook() const override;
    QgsSurveyFieldBookEntry fieldBookEntry( const QgsFeatureId id ) const override;
    int fieldBookCount() const override;

    QList<QgsSurveyPoint> points() const override;
    QgsSurveyPoint point( const QgsFeatureId id ) const override;
    int pointCount() const override;

    QList<QgsSurveyObservationGroup> observationGroups() const override;
    QgsSurveyObservationGroup observationGroup( const QgsFeatureId id ) const override;
    int observationGroupsCount() const override;

    QgsSurveyObservation observation( const QgsFeatureId id ) const override;

    std::variant<QgsSurveyFieldBookEntry, QgsSurveyPoint, QgsSurveyObservationGroup, QgsSurveyObservation> item( const QgsFeatureId id ) const override;

    QgsSdalProvider *clone() const override;

  private:
    QgsSdalProvider( QgsSdalProvider *provider );

    std::shared_ptr<sdal::Dataset> mDataset;

    QHash<QgsFeatureId, QgsSurveyFieldBookEntry> mFieldBook;
    QHash<QgsFeatureId, QgsSurveyPoint> mPoints;
    QHash<QgsFeatureId, QgsSurveyObservationGroup> mObservationGroups;
    QHash<QgsFeatureId, QgsSurveyObservation> mObservations;
};

class QgsSdalProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT

  public:
    QgsSdalProviderMetadata();
    QIcon icon() const override;
    QgsSdalProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() ) override;
    QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    int priorityForUri( const QString &uri ) const override;
    QList<Qgis::LayerType> validLayerTypesForUri( const QString &uri ) const override;
    QString filters( Qgis::FileFilterType type ) override;
    ProviderCapabilities providerCapabilities() const override;
    QList<Qgis::LayerType> supportedLayerTypes() const override;
    void initProvider() override;

  private:
    static QString sFilterString;
    static QStringList sExtensions;
    void buildSupportedSurveyFileFilterAndExtensions();
};
