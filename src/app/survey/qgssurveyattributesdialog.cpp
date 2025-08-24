#include "qgsdockablewidgethelper.h"
#include "qgsdockwidget.h"
#include "qgssurveyattributesdialog.h"
#include "qgssurveylayer.h"
#include "qgssurveyobservationmodel.h"
#include "qgsunittypes.h"

#include <qgisapp.h>


class QgsSurveyFieldBookModel : public QAbstractListModel
{
  public:
    explicit QgsSurveyFieldBookModel( QgsSurveyLayer *layer, QObject *parent = nullptr )
      : QAbstractListModel( parent ), mLayer( layer )
    {
      for ( const QgsSurveyFieldBookEntry &e : mLayer->dataProvider()->fieldBook() )
      {
        mIdOrder.append( e.id );
      }
    }

    QModelIndex indexFor( const QgsFeatureId &id, const int column = 0 ) const
    {
      Q_ASSERT( mIdOrder.contains( id ) && column > 0 && column < columnCount() );
      return index( mIdOrder.indexOf( id ), column );
    }
    QgsFeatureId idFor( const QModelIndex &index ) const
    {
      return mIdOrder.at( index.row() );
    }

    int rowCount( const QModelIndex & = QModelIndex() ) const override
    {
      return mIdOrder.size();
    }
    int columnCount( const QModelIndex & = QModelIndex() ) const override
    {
      return 6;
    }
    QVariant data( const QModelIndex &index, int role ) const override
    {
      const QgsSurveyFieldBookEntry &entry = mLayer->dataProvider()->fieldBookEntry( mIdOrder.at( index.row() ) );
      if ( role == Qt::DisplayRole )
      {
        switch ( index.column() )
        {
          case 0:
            return entry.at;
          case 1:
            return entry.observations.has_value() ? mLayer->dataProvider()->point( entry.observations->station ).name : QString();
          case 2:
            return entry.observations.has_value() ? entry.observations->observations.size() : QVariant();
          case 3:
            return entry.points.isEmpty() ? QVariant() : entry.points.size();
          case 4:
            return entry.metadata.keys().join( ", " );
          case 5:
            return entry.text;
        }
      }
      return QVariant();
    }
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override
    {
      if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
      {
        switch ( section )
        {
          case 0:
            return tr( "At" );
          case 1:
            return tr( "Station" );
          case 2:
            return tr( "Observations" );
          case 3:
            return tr( "Points" );
          case 4:
            return tr( "Metadata" );
          case 5:
            return tr( "Text" );
        }
      }
      return QVariant();
    }

  private:
    QgsSurveyLayer *mLayer;
    QVector<QgsFeatureId> mIdOrder;
};

class QgsSurveyPointModel : public QAbstractListModel
{
  public:
    explicit QgsSurveyPointModel( QgsSurveyLayer *layer, QObject *parent = nullptr )
      : QAbstractListModel( parent ), mLayer( layer )
    {
      for ( const QgsSurveyPoint &p : mLayer->dataProvider()->points() )
      {
        mIdOrder.append( p.id );
      }
    }

    QModelIndex indexFor( const QgsFeatureId &id, const int column = 0 ) const
    {
      Q_ASSERT( mIdOrder.contains( id ) && column >= 0 && column < columnCount() );
      return index( mIdOrder.indexOf( id ), column );
    }
    QgsFeatureId idForIndex( const QModelIndex &index ) const
    {
      return mIdOrder.at( index.row() );
    }

    int rowCount( const QModelIndex & = QModelIndex() ) const override
    {
      return mIdOrder.size();
    }
    int columnCount( const QModelIndex & = QModelIndex() ) const override
    {
      return 8;
    }
    QVariant data( const QModelIndex &index, int role ) const override
    {
      const QgsSurveyPoint &point = mLayer->dataProvider()->point( mIdOrder.at( index.row() ) );
      if ( role == Qt::DisplayRole )
      {
        switch ( index.column() )
        {
          case 0:
            return point.name;
          case 1:
            if ( point.type.has_value() )
            {
              switch ( point.type.value() )
              {
                case QgsSurveyPointType::Fix:
                  return tr( "Fix" );
                case QgsSurveyPointType::New:
                  return tr( "New" );
              }
            }
            else
            {
              return QVariant();
            }
        }
        if ( !point.coordinates.isEmpty() )
        {
          const auto [source, coord] = point.bestCoordinate();
          switch ( index.column() )
          {
            case 2:
              switch ( source )
              {
                case QgsSurveyCoordinateSource::KeyedIn:
                  return tr( "Keyed in" );
                case QgsSurveyCoordinateSource::Adjusted:
                  return tr( "Adjusted" );
                case QgsSurveyCoordinateSource::Approximated:
                  return tr( "Approximated" );
                case QgsSurveyCoordinateSource::ReductionInInstrument:
                  return tr( "Reduced in instrument" );
              }
            case 3:
              return coord.xy.has_value() ? QStringLiteral( "%1 %2" ).arg( coord.xy->x, 0, 'f' ).arg( QgsUnitTypes::toAbbreviatedString( coord.xy->unit ) ) : QVariant();
            case 4:
              return coord.xy.has_value() ? QStringLiteral( "%1 %2" ).arg( coord.xy->y, 0, 'f' ).arg( QgsUnitTypes::toAbbreviatedString( coord.xy->unit ) ) : QVariant();
            case 5:
              return coord.h.has_value() ? QStringLiteral( "%1 %2" ).arg( coord.h->value, 0, 'f' ).arg( QgsUnitTypes::toAbbreviatedString( coord.h->unit ) ) : QVariant();
            case 6:
              return coord.xy.has_value() && coord.xy->uncertainty.has_value() ? QStringLiteral( "%1 %2" ).arg( coord.xy->uncertainty.value(), 0, 'f' ).arg( QgsUnitTypes::toAbbreviatedString( coord.xy->unit ) ) : QVariant();
            case 7:
              return coord.h.has_value() && coord.h->uncertainty.has_value() ? QStringLiteral( "%1 %2" ).arg( coord.h->uncertainty.value(), 0, 'f' ).arg( QgsUnitTypes::toAbbreviatedString( coord.h->unit ) ) : QVariant();
          }
        }
      }
      return QVariant();
    }
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override
    {
      if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
      {
        switch ( section )
        {
          case 0:
            return tr( "Name" );
          case 1:
            return tr( "Type" );
          case 2:
            return tr( "Coordinate" );
          case 3:
            return tr( "X" );
          case 4:
            return tr( "Y" );
          case 5:
            return tr( "Z" );
          case 6:
            return tr( "σXY" );
          case 7:
            return tr( "σZ" );
        }
      }
      return QVariant();
    }

  private:
    QgsSurveyLayer *mLayer;
    QVector<QgsFeatureId> mIdOrder;
};

QgsSurveyAttributesDialog::QgsSurveyAttributesDialog( QgsSurveyLayer *layer, QWidget *parent, Qt::WindowFlags flags, bool *initiallyDocked )
  : QgsOptionsDialogBase( QString(), parent, flags ), mLayer( layer )
{
  setObjectName( QStringLiteral( "QgsSurveyAttributesDialog/" ) + layer->id() );

  setupUi( this );
  initOptionsBase( false );

  connect( mActionToggleEditing, &QAction::toggled, mActionSaveEdits, &QAction::setEnabled );
  connect( mActionToggleEditing, &QAction::toggled, mActionSaveEdits, &QAction::setEnabled );
  connect( mActionToggleEditing, &QAction::toggled, mActionReload, &QAction::setDisabled );

  connect( mActionToggleEditing, &QAction::triggered, this, &QgsSurveyAttributesDialog::mActionToggleEditing_toggled );
  connect( mActionSaveEdits, &QAction::triggered, this, &QgsSurveyAttributesDialog::mActionSaveEdits_triggered );
  connect( mActionReload, &QAction::triggered, this, &QgsSurveyAttributesDialog::mActionReload_triggered );

  setAttribute( Qt::WA_DeleteOnClose );

  layout()->setContentsMargins( 0, 0, 0, 0 );

  QgsSettings settings;
  int size = settings.value( QStringLiteral( "/qgis/toolbarIconSize" ), 16 ).toInt();
  if ( size > 32 )
  {
    size -= 16;
  }
  else if ( size == 32 )
  {
    size = 24;
  }
  else
  {
    size = 16;
  }
  mToolbar->setIconSize( QSize( size, size ) );

  restoreGeometry( settings.value( QStringLiteral( "Windows/SurveyAttributes/geometry" ) ).toByteArray() );

  if ( mLayer )
  {
    connect( mLayer, &QgsSurveyLayer::editingStarted, this, &QgsSurveyAttributesDialog::editingToggled );
    connect( mLayer, &QgsSurveyLayer::editingStopped, this, &QgsSurveyAttributesDialog::editingToggled );
    connect( mLayer, &QgsSurveyLayer::readOnlyChanged, this, &QgsSurveyAttributesDialog::editingToggled );
  }

  connect( this, &QgsSurveyAttributesDialog::saveEdits, this, [] { QgisApp::instance()->saveEdits(); } );

  QgsDockableWidgetHelper::OpeningMode openingMode = QgsDockableWidgetHelper::OpeningMode::RespectSetting;
  if ( initiallyDocked )
    openingMode = *initiallyDocked ? QgsDockableWidgetHelper::OpeningMode::ForceDocked : QgsDockableWidgetHelper::OpeningMode::ForceDialog;
  mDockableWidgetHelper = new QgsDockableWidgetHelper( windowTitle(), this, QgisApp::instance(), QStringLiteral( "attribute-table" ), QStringList(), openingMode, true, Qt::BottomDockWidgetArea );
  toggleShortcuts( !mDockableWidgetHelper->isDocked() );
  connect( mDockableWidgetHelper, &QgsDockableWidgetHelper::closed, this, [this]() {
    close();
  } );
  connect( mDockableWidgetHelper, &QgsDockableWidgetHelper::dockModeToggled, this, [this]( bool docked ) {
    if ( docked )
    {
      toggleShortcuts( mDockableWidgetHelper->dockWidget()->isFloating() );
    }
    else
    {
      toggleShortcuts( true );
    }
  } );

  mActionDockUndock = mDockableWidgetHelper->createDockUndockAction( tr( "Dock Attribute Table" ), this );
  mToolbar->addAction( mActionDockUndock );

  mActionToggleEditing->blockSignals( true );
  mActionToggleEditing->setCheckable( true );
  mActionToggleEditing->setChecked( mLayer && mLayer->isEditable() );
  mActionToggleEditing->blockSignals( false );

  mActionSaveEdits->setEnabled( mActionToggleEditing->isEnabled() && mLayer && mLayer->isEditable() && mLayer->isModified() );
  mActionReload->setEnabled( mLayer && !mLayer->isEditable() );

  Qgis::SurveyProviderCapabilities capabilities = ( mLayer && mLayer->dataProvider() ) ? mLayer->dataProvider()->capabilities() : Qgis::SurveyProviderCapabilities();
  const bool canReload = capabilities & Qgis::SurveyProviderCapability::ReloadData;

  if ( !canReload )
  {
    mToolbar->removeAction( mActionReload );
  }

  mModelFieldBook = new QgsSurveyFieldBookModel( layer, this );
  mProxyModelFieldBook = new QSortFilterProxyModel( this );
  mProxyModelFieldBook->setSourceModel( mModelFieldBook );
  mProxyModelFieldBook->setFilterKeyColumn( 1 );
  mFieldBookListView->setModel( mProxyModelFieldBook );
  connect( mFieldBookFilterEdit, &QLineEdit::textChanged, mProxyModelFieldBook, &QSortFilterProxyModel::setFilterWildcard );

  mModelPoints = new QgsSurveyPointModel( layer, this );
  mProxyModelPoints = new QSortFilterProxyModel( this );
  mProxyModelPoints->setSourceModel( mModelPoints );
  mProxyModelPoints->setFilterKeyColumn( 0 );
  mPointsView->setModel( mProxyModelPoints );
  connect( mPointsFilterEdit, &QLineEdit::textChanged, mProxyModelPoints, &QSortFilterProxyModel::setFilterWildcard );
  connect( mPointsView->selectionModel(), &QItemSelectionModel::selectionChanged, [this]( const QItemSelection &selected, const QItemSelection &deselected ) {
    QgsFeatureIds selectedIds, deselectedIds;

    for ( const QModelIndex &index : selected.indexes() )
    {
      selectedIds.insert( mModelPoints->idForIndex( mProxyModelPoints->mapToSource( index ) ) );
    }
    for ( const QModelIndex &index : deselected.indexes() )
    {
      deselectedIds.insert( mModelPoints->idForIndex( mProxyModelPoints->mapToSource( index ) ) );
    }

    mLayer->modifySelection( selectedIds, deselectedIds );
  } );
  connect( mLayer, &QgsSurveyLayer::selectionChanged, this, [this]( const QgsFeatureIds &selectedIds, const QgsFeatureIds &deselectedIds ) {
    QItemSelection selected, deselected;

    for ( const QgsFeatureId &id : selectedIds )
    {
      const QModelIndex index = mModelPoints->indexFor( id );
      if ( !index.isValid() )
      {
        continue;
      }
      selected.append( QItemSelectionRange( mProxyModelPoints->mapFromSource( index ) ) );
    }
    mPointsView->selectionModel()->select( selected, QItemSelectionModel::Select | QItemSelectionModel::Rows );
    for ( const QgsFeatureId &id : deselectedIds )
    {
      const QModelIndex index = mModelPoints->indexFor( id );
      if ( !index.isValid() )
      {
        continue;
      }
      deselected.append( QItemSelectionRange( mProxyModelPoints->mapFromSource( index ) ) );
    }
    mPointsView->selectionModel()->select( deselected, QItemSelectionModel::Deselect | QItemSelectionModel::Rows );
  } );

  mModelObservations = new QgsSurveyObservationModel( layer, this );
  mMeasurementsView->setModel( mModelObservations );
  connect( mMeasurementsView->selectionModel(), &QItemSelectionModel::selectionChanged, [this]( const QItemSelection &selected, const QItemSelection &deselected ) {
    QgsFeatureIds selectedIds, deselectedIds;

    for ( const QModelIndex &index : selected.indexes() )
    {
      selectedIds.insert( mModelObservations->idForIndex( index ) );
    }
    for ( const QModelIndex &index : deselected.indexes() )
    {
      deselectedIds.insert( mModelObservations->idForIndex( index ) );
    }

    mLayer->modifySelection( selectedIds, deselectedIds );
  } );
  connect( mLayer, &QgsSurveyLayer::selectionChanged, this, [this]( const QgsFeatureIds &selectedIds, const QgsFeatureIds &deselectedIds ) {
    QItemSelection selected, deselected;

    for ( const QgsFeatureId &id : selectedIds )
    {
      const QModelIndex index = mModelObservations->indexFor( id );
      if ( !index.isValid() )
      {
        continue;
      }
      selected.append( QItemSelectionRange( index ) );
      mMeasurementsView->expand( index );
      if ( mModelObservations->rowCount( index ) == 1 )
      {
        mMeasurementsView->expand( mModelObservations->index( 0, 0, index ) );
      }
    }
    mMeasurementsView->selectionModel()->select( selected, QItemSelectionModel::Select | QItemSelectionModel::Rows );
    for ( const QgsFeatureId &id : deselectedIds )
    {
      const QModelIndex index = mModelObservations->indexFor( id );
      if ( !index.isValid() )
      {
        continue;
      }
      deselected.append( QItemSelectionRange( index ) );
    }
    mMeasurementsView->selectionModel()->select( selected, QItemSelectionModel::Deselect | QItemSelectionModel::Rows );
  } );

  editingToggled();
  connect( mLayer, &QObject::destroyed, this, &QWidget::close );
}

QgsSurveyAttributesDialog::~QgsSurveyAttributesDialog()
{
  delete mDockableWidgetHelper;
}

QDomElement QgsSurveyAttributesDialog::writeXml( QDomDocument &document )
{
  QDomElement element = document.createElement( QStringLiteral( "surveyAttributesDialog" ) );
  mDockableWidgetHelper->writeXml( element );

  element.setAttribute( QStringLiteral( "page" ), mOptionsStackedWidget->currentWidget()->objectName() );

  return element;
}

void QgsSurveyAttributesDialog::readXml( const QDomElement &element )
{
  mDockableWidgetHelper->readXml( element );

  QWidget *currentWidget = mOptionsStackedWidget->findChild<QWidget *>( element.attribute( QStringLiteral( "page" ) ) );
  mOptionsListWidget->setCurrentRow( mOptionsStackedWidget->indexOf( currentWidget ) );
}

void QgsSurveyAttributesDialog::editingToggled()
{
  const bool isEditable = mLayer->isEditable();
  mActionToggleEditing->blockSignals( true );
  mActionToggleEditing->setChecked( isEditable );
  mActionSaveEdits->setEnabled( isEditable && mLayer->isModified() );
  mActionReload->setEnabled( !isEditable );
  mActionToggleEditing->blockSignals( false );
}

void QgsSurveyAttributesDialog::mActionToggleEditing_toggled( bool )
{
  if ( !mLayer )
    return;

  if ( !QgisApp::instance()->toggleEditing( mLayer ) )
  {
    // restore gui state if toggling was canceled or layer commit/rollback failed
    editingToggled();
  }
}

void QgsSurveyAttributesDialog::mActionSaveEdits_triggered()
{
  QgisApp::instance()->saveEdits( mLayer, true, true );
}

void QgsSurveyAttributesDialog::mActionReload_triggered()
{
  mLayer->reload();
}

void QgsSurveyAttributesDialog::keyPressEvent( QKeyEvent *event )
{
  QDialog::keyPressEvent( event );

  if ( ( event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete ) && mActionDeleteSelected->isEnabled() )
  {
    // When docked, let the main window handle the key events
    if ( !mDockableWidgetHelper->dockWidget() || mDockableWidgetHelper->dockWidget()->isFloating() )
    {
      QgisApp::instance()->deleteSelected( mLayer, this );
    }
  }
}

void QgsSurveyAttributesDialog::toggleShortcuts( bool enable )
{}
