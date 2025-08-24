#pragma once

#include <QDialog>
#include <QPointer>

#include "qgis_app.h"
#include "ui_qgssurveyattributesdialog.h"
#include "qgsoptionsdialogbase.h"

class QgsSurveyObservationModel;
class QgsMapLayer;
class QgsDockableWidgetHelper;
class QgsSurveyLayer;
class QgsSurveyFieldBookModel;
class QgsSurveyPointModel;
class QDomDocument;
class QDomElement;

class APP_EXPORT QgsSurveyAttributesDialog : public QgsOptionsDialogBase, private Ui::QgsSurveyAttributesDialog
{
    Q_OBJECT

  public:
    QgsSurveyAttributesDialog( QgsSurveyLayer *layer, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::Window, bool *initiallyDocked = nullptr );
    ~QgsSurveyAttributesDialog() override;

    QDomElement writeXml( QDomDocument &document );
    void readXml( const QDomElement &element );

    QgsDockableWidgetHelper *dockableWidgetHelper() { return mDockableWidgetHelper; }

  public slots:
    void editingToggled();

  private slots:
    void mActionToggleEditing_toggled( bool );
    void mActionSaveEdits_triggered();
    void mActionReload_triggered();

  signals:
    void saveEdits( QgsMapLayer *layer );

  protected:
    void keyPressEvent( QKeyEvent *event ) override;

    QDialog *mDialog = nullptr;

    QPointer<QgsSurveyLayer> mLayer = nullptr;

    QAction *mActionDockUndock = nullptr;
    QgsDockableWidgetHelper *mDockableWidgetHelper = nullptr;

    void toggleShortcuts( bool enable );

    QgsSurveyFieldBookModel *mModelFieldBook;
    QSortFilterProxyModel *mProxyModelFieldBook;
    QgsSurveyPointModel *mModelPoints;
    QSortFilterProxyModel *mProxyModelPoints;
    QgsSurveyObservationModel *mModelObservations;
};
