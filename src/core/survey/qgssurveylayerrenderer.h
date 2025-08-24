#ifndef QGSSURVEYLAYERRENDERER_H
#define QGSSURVEYLAYERRENDERER_H

#include "qgsmaplayerrenderer.h"
#include "qgssurveylayer.h"

class QgsMarkerSymbol;

class CORE_EXPORT QgsSurveyLayerRenderer : public QgsMapLayerRenderer
{
  public:
    explicit QgsSurveyLayerRenderer( QgsSurveyLayer *layer, QgsRenderContext &context );
    ~QgsSurveyLayerRenderer();

    bool render() override;
    Qgis::MapLayerRendererFlags flags() const override;
    QgsFeedback *feedback() const override { return mFeedback.get(); }

  private:
    std::unique_ptr<QgsSurveyDataProvider> mDataProvider;
    std::unique_ptr<QgsFeedback> mFeedback;

    std::unique_ptr<QgsMarkerSymbol> mSymbolFix;
    std::unique_ptr<QgsMarkerSymbol> mSymbolStationFix;
    std::unique_ptr<QgsMarkerSymbol> mSymbolStation;
    std::unique_ptr<QgsMarkerSymbol> mSymbolPoint;
};

#endif // QGSSURVEYLAYERRENDERER_H
