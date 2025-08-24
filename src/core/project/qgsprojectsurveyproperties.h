#pragma once

#include "qgsmaplayerref.h"
#include <QObject>

class QDomElement;
class QDomDocument;
class QgsReadWriteContext;
class QgsProject;
class QgsCoordinateReferenceSystem;
class QgsVectorLayer;
class QgsVectorDataProvider;

class CORE_EXPORT QgsAbstractKnownPointsProvider
{
  public:
    virtual ~QgsAbstractKnownPointsProvider();

    QgsAbstractKnownPointsProvider &operator=( const QgsAbstractKnownPointsProvider &other ) = delete;

    virtual bool equals( const QgsAbstractKnownPointsProvider *other ) const = 0;
    virtual void resolveReferences( const QgsProject *project );
    virtual bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) = 0;
    virtual QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const = 0;
    virtual QString type() const = 0;
    virtual QgsAbstractKnownPointsProvider *clone() const = 0 SIP_FACTORY;
    virtual void prepare() = 0 SIP_FACTORY;
    virtual QgsCoordinateReferenceSystem crs() const = 0;

    virtual QHash<QString, QgsPoint> points( const QSet<QString> &ids ) const = 0;

  protected:
    QgsAbstractKnownPointsProvider() = default;

#ifndef SIP_RUN
    QgsAbstractKnownPointsProvider( const QgsAbstractKnownPointsProvider &other );
#endif

    /**
     * Writes common properties to a DOM \a element.
     */
    void writeCommonProperties( QDomElement &element, const QgsReadWriteContext &context ) const;

    /**
     * Reads common properties from a DOM \a element.
     */
    void readCommonProperties( const QDomElement &element, const QgsReadWriteContext &context );

  private:
#ifdef SIP_RUN
    QgsAbstractKnownPointsProvider( const QgsAbstractKnownPointsProvider &other );
#endif
};
class CORE_EXPORT QgsNoKnownPointsProvider : public QgsAbstractKnownPointsProvider
{
  public:
    QgsNoKnownPointsProvider() = default;

    QString type() const override { return QStringLiteral( "no" ); }
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsNoKnownPointsProvider *clone() const override SIP_FACTORY;
    void prepare() override;
    bool equals( const QgsAbstractKnownPointsProvider *other ) const override;

    QHash<QString, QgsPoint> points( const QSet<QString> & ) const override { return {}; }
};
class CORE_EXPORT QgsVectorKnownPointsProvider : public QgsAbstractKnownPointsProvider
{
  public:
    QgsVectorKnownPointsProvider() = default;

#ifndef SIP_RUN
    const QgsVectorKnownPointsProvider *operator=( const QgsVectorKnownPointsProvider &other ) = delete;
#endif

    QString type() const override { return QStringLiteral( "vector" ); }
    void resolveReferences( const QgsProject *project ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsVectorKnownPointsProvider *clone() const override SIP_FACTORY;
    void prepare() override;
    bool equals( const QgsAbstractKnownPointsProvider *other ) const override;

    void setLayer( QgsVectorLayer *layer );
    QgsVectorLayer *layer() const;

    void setKeyAttribute( const QString &attribute ) { mKeyAttribute = attribute; }
    QString keyAttribute() const { return mKeyAttribute; }

    QHash<QString, QgsPoint> points( const QSet<QString> & ) const override;

  private:
    QgsVectorKnownPointsProvider( const QgsVectorKnownPointsProvider &other );

    QString mKeyAttribute;

    _LayerRef<QgsVectorLayer> mVectorLayer;
    std::unique_ptr< QgsVectorLayer > mClonedVectorLayer;
};

class CORE_EXPORT QgsProjectSurveyProperties : public QObject
{
    Q_OBJECT

  public:
    explicit QgsProjectSurveyProperties( QObject *parent SIP_TRANSFERTHIS = nullptr );

    void reset();

    void resolveReferences( const QgsProject *project );

    bool readXml( const QDomElement &element, const QgsReadWriteContext &context );
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const;

    QgsAbstractKnownPointsProvider *knownPointsProvider() { return mKnownPointsProvider.get(); }
    void setKnownPointsProvider( QgsAbstractKnownPointsProvider *provider SIP_TRANSFER );

  signals:
    void changed();

  private:
    std::unique_ptr<QgsAbstractKnownPointsProvider> mKnownPointsProvider;
};
