#include "WizardPageCinderBlocks.h"
#include "CinderBlockManager.h"
#include "ui_WizardPageCinderBlocks.h"

#include <QPainter>
#include <QStandardItemModel>
#include <iostream>

static const int    ICON_SIZE = 36;
static const QColor TOP_COLOR( QColor( 247, 247, 247 ) );
static const QColor BOTTOM_COLOR( QColor( 210, 210, 210 ) );
static const QColor SELECTED_TOP_COLOR( QColor( 100, 100, 100 ) );
static const QColor SELECTED_BOTTOM_COLOR( QColor( 64, 64, 64 ) );
static const QColor INSTALLED_TOP_COLOR( QColor( 102, 159, 218 ) );
static const QColor INSTALLED_BOTTOM_COLOR( QColor( 27, 106, 186 ) );
static const QColor SELECTED_INSTALLED_TOP_COLOR( INSTALLED_TOP_COLOR.darker( 150 ) );
static const QColor SELECTED_INSTALLED_BOTTOM_COLOR( INSTALLED_BOTTOM_COLOR.darker( 150 ) );
// text colors
static const QColor PRIMARY_COLOR( QColor( 235, 235, 235, 255 ) );
static const QColor SHADOW_COLOR( QColor( 245, 245, 245, 220 ) );
static const QColor HIGHLIGHTED_COLOR( QColor( 70, 70, 70 ) );
static const QColor HIGHLIGHTED_SHADOW_COLOR( QColor( 48, 48, 48, 220 ) );


void CinderBlockItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
		if( option.state & QStyle::State_Selected )
			painter->fillRect(option.rect, option.palette.highlight());

		QString title = index.data(Qt::DisplayRole).toString();
		QString description = index.data(Qt::UserRole + 1).toString();
		bool installed = index.data(Qt::UserRole + 2).toInt() > CinderBlock::INSTALL_NONE;
		bool selected = option.state & QStyle::State_Selected;
		QString iconPath = index.data(Qt::UserRole + 3).toString();
		painter->save();
				
		QLinearGradient grad( option.rect.left(), option.rect.top(), option.rect.left(), option.rect.bottom() );
		if( selected ) {
			if( installed ) {
				grad.setColorAt( 0, SELECTED_INSTALLED_TOP_COLOR );
				grad.setColorAt( 1, SELECTED_INSTALLED_BOTTOM_COLOR );
			}
			else {
				grad.setColorAt( 0, SELECTED_TOP_COLOR );
				grad.setColorAt( 1, SELECTED_BOTTOM_COLOR );
			}
			painter->fillRect( option.rect, QBrush( grad ) );
			QPen pen( QColor( 255, 255, 255 ) );
			QRect r = option.rect.adjusted(3, 3, -3, -3);
			pen.setWidth( 5 );
			painter->setPen( pen );
			//painter->drawRect( option.rect );			
		}
		else {
			if( installed ) {
				grad.setColorAt( 0, INSTALLED_TOP_COLOR );
				grad.setColorAt( 1, INSTALLED_BOTTOM_COLOR );
			}
			else {
				grad.setColorAt( 0, TOP_COLOR );
				grad.setColorAt( 1, BOTTOM_COLOR );
			}
			painter->fillRect( option.rect.adjusted( 0, 2, 0, -2 ), QBrush( grad ) );
		}

		const QIcon &icon = CinderBlockManager::getIcon( iconPath );
		icon.paint( painter, option.rect.left() + 6, option.rect.top() + 6, ICON_SIZE, ICON_SIZE );

		QRect r = option.rect.adjusted(50, 0, 0, 0);

#ifdef Q_OS_WIN32
		painter->setFont( QFont( "Arial", 10, QFont::Bold ) );
#else
		painter->setFont( QFont( "Lucida Grande", 14, QFont::Bold ) );
#endif
		// draw "shadow" of name
		if( selected || installed )
			painter->setPen( QPen( HIGHLIGHTED_SHADOW_COLOR ) );
		else
			painter->setPen( QPen( SHADOW_COLOR ) );
		painter->drawText(r.left() + 1, r.top() + 10 + 1, r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft|Qt::TextWordWrap, title );
		
		// draw name
		if( selected || installed )
			painter->setPen( QPen( PRIMARY_COLOR ) );
		else
			painter->setPen( QPen( HIGHLIGHTED_COLOR ) );
		painter->drawText(r.left(), r.top() + 10, r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft|Qt::TextWordWrap, title );
		
		// description
#ifdef Q_OS_WIN32
		painter->setFont( QFont( "Arial", 9 ) );
#else
		painter->setFont( QFont( "Lucida Grande", 10 ) );
#endif
		// draw "shadow" of description
		if( selected || installed )
			painter->setPen( QPen( HIGHLIGHTED_SHADOW_COLOR ) );
		else
			painter->setPen( QPen( SHADOW_COLOR ) );
		painter->drawText(r.left() + 1, r.top() + 26 + 1, 260, r.height(), Qt::AlignTop|Qt::AlignLeft|Qt::TextWordWrap, description );
		
		// draw description
		if( selected || installed )
			painter->setPen( QPen( PRIMARY_COLOR ) );
		else
			painter->setPen( QPen( HIGHLIGHTED_COLOR ) );
		painter->drawText(r.left(), r.top() + 26, 260, r.height(), Qt::AlignTop|Qt::AlignLeft|Qt::TextWordWrap, description );

		painter->restore();
}

WizardPageCinderBlocks::WizardPageCinderBlocks( MainWizard *parent ) :
	QWizardPage( parent ), mParent( parent ),
	ui(new Ui::WizardPageCinderBlocks)
{
	ui->setupUi(this);
	ui->cinderBlockList->setItemDelegate( new CinderBlockItemDelegate );
	connect( ui->cinderBlockList, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()) );
	connect( ui->cinderBlockList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)) );
	connect( ui->installTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(installTypeChanged()) );

	QPalette p = ui->cinderBlockList->palette();
	p.setBrush( QPalette::Base, QBrush() );
	p.setColor( QPalette::WindowText, QColor( 0, 0, 0, 0 ) );
	ui->cinderBlockList->setPalette( p );

#ifdef Q_OS_WIN32
	ui->nameId->setFont( QFont( "Arial", 8 ) );
	ui->nameContents->setFont( QFont( "Arial", 11, QFont::Bold ) );
#endif
}

WizardPageCinderBlocks::~WizardPageCinderBlocks()
{
	delete ui;
}

void WizardPageCinderBlocks::setCinderLocation( const QString &/*path*/ )
{
	updateCinderBlockList();
}

void WizardPageCinderBlocks::updateCinderBlockList()
{
	ui->cinderBlockList->clear();

	for( QList<CinderBlock>::ConstIterator blockIt = mParent->getCinderBlocks().begin(); blockIt != mParent->getCinderBlocks().end(); ++blockIt ) {
		QListWidgetItem *item = new QListWidgetItem();
		item->setData( Qt::DisplayRole, blockIt->getName() );
		item->setData( Qt::UserRole + 1, blockIt->getId() );
		item->setData( Qt::UserRole + 2, QVariant( (int)blockIt->getInstallType() ) );
		item->setData( Qt::UserRole + 3, blockIt->getIconPath() );
		ui->cinderBlockList->addItem( item );
	}
	
	if( ui->cinderBlockList->count() > 0 ) {
		ui->cinderBlockList->setCurrentItem( ui->cinderBlockList->item( 0 ) );
	}
	
	selectionChanged(); // force a refresh to the install menu

	if( mParent->getCinderBlockErrorList().empty() ) {
		ui->showErrorsButton->hide();
		ui->listWarningsLayout->removeWidget( ui->showErrorsButton );
	}
	else {
		ui->showErrorsButton->show();
		ui->listWarningsLayout->addWidget( ui->showErrorsButton );
	}
}

CinderBlock* WizardPageCinderBlocks::getCurrentCinderBlock()
{
	size_t idx = ui->cinderBlockList->currentRow();
	for( QList<CinderBlock>::Iterator cIt = mParent->getCinderBlocks().begin(); cIt != mParent->getCinderBlocks().end(); ++cIt ) {
		if( idx-- == 0 )
			return &(*cIt);
	}
	
	// oh great - our current selection has gone off the deep end
	if( mParent->getCinderBlocks().empty() )
		return NULL;
	else {
		ui->cinderBlockList->setCurrentItem( ui->cinderBlockList->item( 0 ) );
		return &*(mParent->getCinderBlocks().begin());
	}
}

void WizardPageCinderBlocks::selectionChanged()
{
	const CinderBlock *currentCinderBlock = getCurrentCinderBlock();

	// delete all existing items after the install combo	
	while( ui->formLayout->count() > 4 ) {
		QLayoutItem *item = ui->formLayout->takeAt( 4 );
		delete item->widget();
		delete item;
	}
	
	if( currentCinderBlock ) {
		QString fullName = currentCinderBlock->getName();
		const QIcon &icon = CinderBlockManager::getIcon( currentCinderBlock->getIconPath() );
		ui->nameLabel->setPixmap( icon.pixmap( 48 ) );
		ui->nameContents->setText( fullName );
		ui->nameId->setText( currentCinderBlock->getId() );
		ui->installTypeCombo->setCurrentIndex( currentCinderBlock->getInstallType() - CinderBlock::INSTALL_NONE );
		if( ! currentCinderBlock->getAuthor().isEmpty() ) {
			ui->formLayout->addRow( (QWidget*)new QLabel( QString::fromUtf8("<b>Author:</b>") ), (QWidget*)new QLabel( currentCinderBlock->getAuthor() ) );
		}

		// Block URL
		if( ! currentCinderBlock->getBlockUrl().isEmpty() ) {
			QString blockUrl = currentCinderBlock->getBlockUrl();
			QString strippedBlockUrl = blockUrl.replace( "http://", "", Qt::CaseInsensitive );
			strippedBlockUrl = strippedBlockUrl.replace( "https://", "", Qt::CaseInsensitive );
			QLabel *urlLabel = new QLabel( QString("<a href=\"") + blockUrl + "\">" + strippedBlockUrl + "</a>" );
			urlLabel->setWordWrap( true );
			urlLabel->setOpenExternalLinks( true );
			ui->formLayout->addRow( (QWidget*)new QLabel( QString::fromUtf8("<b>URL:</b>") ), urlLabel );
		}

		// Git
		if( ! currentCinderBlock->getGitUrl().isEmpty() ) {		
			QString convertedUrl = currentCinderBlock->getGitUrl();
			convertedUrl.replace( "git://", "http://", Qt::CaseInsensitive );
			QLabel *gitLabel = new QLabel( QString("<a href=") + convertedUrl + ">" + currentCinderBlock->getGitUrl() + "</a>" );
			gitLabel->setWordWrap( true );
			gitLabel->setOpenExternalLinks( true );
			ui->formLayout->addRow( (QWidget*)new QLabel( QString::fromUtf8("<b>Git:</b>") ), gitLabel );
		}

		// Library
		if( ! currentCinderBlock->getLibraryUrl().isEmpty() ) {
			QString libraryUrl = currentCinderBlock->getLibraryUrl();
			QString strippedLibraryUrl = libraryUrl.replace( "http://", "", Qt::CaseInsensitive );
			strippedLibraryUrl = strippedLibraryUrl.replace( "https://", "", Qt::CaseInsensitive );
			QLabel *library = new QLabel( QString("<a href=\"") + currentCinderBlock->getLibraryUrl() + "\">" + strippedLibraryUrl + "</a>" );
			library->setWordWrap( true );
			library->setOpenExternalLinks( true );
			ui->formLayout->addRow( (QWidget*)new QLabel( QString::fromUtf8("<b>Library:</b>") ), library );
		}

		// Version
		if( ! currentCinderBlock->getVersion().isEmpty() ) {
			QLabel *version = new QLabel( currentCinderBlock->getVersion() );
			version->setWordWrap( true );
			ui->formLayout->addRow( (QWidget*)new QLabel( QString::fromUtf8("<b>Version:</b>") ), version );
		}

		// License
		if( ! currentCinderBlock->getLicense().isEmpty() ) {
			QLabel *license = new QLabel( currentCinderBlock->getLicense() );
			license->setWordWrap( true );
			ui->formLayout->addRow( (QWidget*)new QLabel( QString::fromUtf8("<b>License:</b>") ), license );
		}

		// Summary
		if( ! currentCinderBlock->getDescription().isEmpty() ) {		
			QLabel *description = new QLabel( QString("\n") + currentCinderBlock->getDescription() );
			description->setWordWrap( true );
			ui->formLayout->addRow( (QWidget*)new QLabel( QString::fromUtf8("") ), description );
		}

		if( currentCinderBlock->isRequired() ) {
			QStandardItemModel *model = (QStandardItemModel*)ui->installTypeCombo->model();
			model->item( 0 )->setEnabled( false );
			ui->installTypeCombo->setItemText( 0, "Required" );
		}
		else {
			QStandardItemModel *model = (QStandardItemModel*)ui->installTypeCombo->model();
			model->item( 0 )->setEnabled( true );
			ui->installTypeCombo->setItemText( 0, "None" );
		}

		bool enableGitSubmodule = mParent->shouldCreateGitRepo() && ( ! currentCinderBlock->getGitUrl().isEmpty() );
		QStandardItemModel *model = (QStandardItemModel*)ui->installTypeCombo->model();
		model->item( CinderBlock::INSTALL_GIT_SUBMODULE )->setEnabled( enableGitSubmodule );
	}
	else {
/*		ui->nameContents->setText( "" );
		ui->installTypeCombo->setCurrentIndex( CinderBlock::INSTALL_NONE );
		ui->authorContents->setText( "" );
		ui->urlContents->setText( "" );
		ui->gitContents->setText( "" );
		ui->descriptionContents->setText( "" );*/
	}
}

void WizardPageCinderBlocks::installTypeChanged()
{
	CinderBlock *currentCinderBlock = getCurrentCinderBlock();
	CinderBlock::InstallType installType = (CinderBlock::InstallType)ui->installTypeCombo->currentIndex();
	currentCinderBlock->setInstallType( installType );
	mParent->refreshRequiredBlocks();

	size_t itemIndex = 0;
	for( QList<CinderBlock>::ConstIterator blockIt = mParent->getCinderBlocks().begin(); blockIt != mParent->getCinderBlocks().end(); ++blockIt, ++itemIndex ) {
		ui->cinderBlockList->item( itemIndex )->setData( Qt::UserRole + 2, QVariant( (int)blockIt->getInstallType() ) );
	}
}

void WizardPageCinderBlocks::itemDoubleClicked(QListWidgetItem*)
{
	// set to copy by default, relative if core
	CinderBlock *currentCinderBlock = getCurrentCinderBlock();
	if( ui->installTypeCombo->currentIndex() != CinderBlock::INSTALL_NONE )
		ui->installTypeCombo->setCurrentIndex( CinderBlock::INSTALL_NONE );
	else if( currentCinderBlock->isCore() )
		ui->installTypeCombo->setCurrentIndex( CinderBlock::INSTALL_REFERENCE );
	else
		ui->installTypeCombo->setCurrentIndex( CinderBlock::INSTALL_COPY );
}

void WizardPageCinderBlocks::on_showErrorsButton_clicked()
{
	ErrorListDialog dlg( this );
	dlg.show( mParent->getCinderBlockErrorList() );
}
