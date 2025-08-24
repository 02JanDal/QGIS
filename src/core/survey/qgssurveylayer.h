#pragma once

#include "qgsmaplayer.h"
#include "qgssurveydataprovider.h"

/**
 * \ingroup core
 *
 * \brief Represents a map layer supporting display of survey data.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.xx
 */
class CORE_EXPORT QgsSurveyLayer : public QgsMapLayer
{
    Q_OBJECT
  public:
    /**
     * Setting options for loading point cloud layers.
     */
    struct LayerOptions
    {
        /**
       * Constructor for LayerOptions with optional \a transformContext.
       */
        explicit LayerOptions( const QgsCoordinateTransformContext &transformContext = QgsCoordinateTransformContext() )
          : transformContext( transformContext )
        {}

        /**
       * Coordinate transform context
       */
        QgsCoordinateTransformContext transformContext;
    };

    explicit QgsSurveyLayer( const QString &uri = QString(), const QString &baseName = QString(), const QString &providerLib = QStringLiteral( "sdal" ), const QgsSurveyLayer::LayerOptions &options = QgsSurveyLayer::LayerOptions() );


  public slots:
    void select( QgsFeatureId featureId );
    Q_INVOKABLE void select( const QgsFeatureIds &featureIds );
    void deselect( QgsFeatureId featureId );
    Q_INVOKABLE void deselect( const QgsFeatureIds &featureIds );
    Q_INVOKABLE void removeSelection();

  public:
    int selectedFeatureCount() const;
    Q_INVOKABLE void selectByIds( const QgsFeatureIds &ids, Qgis::SelectBehavior behavior = Qgis::SelectBehavior::SetSelection );
    Q_INVOKABLE void modifySelection( const QgsFeatureIds &selectIds, const QgsFeatureIds &deselectIds );
    Q_INVOKABLE void invertSelection();
    Q_INVOKABLE void selectAll();

  private:
    QgsFeatureIds mSelectedFeatureIds;
    QgsFeatureIds allFeatureIds() const;

  signals:
    void selectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect );

    void editingStarted();
    void editingStopped();
    void layerModified();
    void readOnlyChanged();
    void pointAdded();
    void pointModified();
    void pointsDeleted();
    void observationGroupAdded();
    void observationGroupModified();
    void observationGroupsDeleted();
    void fieldBookEntryAdded();
    void fieldBookEntryModified();
    void fieldBookEntriesDeleted();

    // QgsMapLayer interface
  public:
    QgsMapLayer *clone() const override;
    QgsSurveyDataProvider *dataProvider() override;
    const QgsSurveyDataProvider *dataProvider() const override;
    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override;
    QgsRectangle extent() const override;
    QString loadDefaultMetadata( bool &resultFlag ) override;
    bool readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) override;
    bool readStyle( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) override;
    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) const override;
    bool writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) const override;
    QString htmlMetadata() const override;
    void reload() override;

  public slots:
    void setTransformContext( const QgsCoordinateTransformContext &transformContext ) override;

  protected:
    bool readXml( const QDomNode &layerNode, QgsReadWriteContext &context ) override;
    bool writeXml( QDomNode &layerNode, QDomDocument &doc, const QgsReadWriteContext &context ) const override;

  private slots:
    void setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags ) override;

  private:
    std::unique_ptr<QgsSurveyDataProvider> mDataProvider;
    LayerOptions mLayerOptions;
};
