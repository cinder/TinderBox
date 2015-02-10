#include "WizardPageMain.h"
#include "MainWizard.h"
#include "Preferences.h"
#include "FirstTimeDlg.h"
#include "ErrorList.h"
#include "ui_WizardPageMain.h"

#include <QCloseEvent>
#include <QDebug>
#include <QErrorMessage>
#include <QFileDialog>
#include <QProcess>
#include <QRegExpValidator>
#include <QSettings>
#include <QPainter>
#include <fstream>

#include "GeneratorXcodeMac.h"
#include "GeneratorXcodeIos.h"
#include "Instancer.h"
#include "ProjectTemplateManager.h"
#include "CinderBlockManager.h"
#include "Util.h"
#include "PList.h"
#include "CinderBlock.h"

#include <iostream>

// add to mPlatformConditions too
enum { XCODE_INDEX, XCODE_IOS_INDEX, VC2012_WINRT_INDEX, VC2013_INDEX, NUM_PLATFORMS };

WizardPageMain::WizardPageMain( MainWizard *parent ) :
	QWizardPage(parent),
	ui(new Ui::WizardPageMain), mParent( parent )
{
    // UI
    ui->setupUi( this );
	ui->projectNameLineEdit->setValidator( new QRegExpValidator( QRegExp( "[A-Za-z_][\\w]*" ), ui->projectNameLineEdit ) );
	ui->projectNameLineEdit->setText( "CinderProject" );
	mLocationPaletteOrig = ui->locationLineEdit->palette();

	// setup platform conditions
	QMap<QString,QString> xcodePlatCond; xcodePlatCond["os"] = "macosx";
	mPlatformConditions.push_back( xcodePlatCond );
	QMap<QString,QString> xcodeIosPlatCond; xcodeIosPlatCond["os"] = "ios";
	mPlatformConditions.push_back( xcodeIosPlatCond );
	QMap<QString,QString> vc2012WinrtPlatCond; vc2012WinrtPlatCond["os"] = "winrt"; vc2012WinrtPlatCond["compiler"] = "vc2012";
	mPlatformConditions.push_back( vc2012WinrtPlatCond );
	QMap<QString,QString> vc2013PlatCond; vc2013PlatCond["os"] = "msw"; vc2013PlatCond["compiler"] = "vc2013";
	mPlatformConditions.push_back( vc2013PlatCond );

	updateTemplates();

	ui->locationLineEdit->setText( Preferences::getOutputPath() );
	ui->sourceControlCheckBox->setChecked( Preferences::getCreateGitRepoDefault() );

#if defined Q_OS_MAC
    ui->compilerList->item( XCODE_INDEX )->setSelected( true );
#endif

#if defined Q_OS_WIN
    ui->compilerList->item( VC2013_INDEX )->setSelected( true );
#endif

	// Update controls
    updateCinderVersionsCtrl();
	validateNextButton();
}

WizardPageMain::~WizardPageMain()
{
	delete ui;
}

void WizardPageMain::initializePage()
{
}

void WizardPageMain::updateTemplates()
{
	// save the currently selected template's ID
	QString oldTemplateId;
	int oldTemplateIdx = ui->templateComboBox->currentIndex();
	if( oldTemplateIdx >= 0 ) {
		QVariant curProjVar = ui->templateComboBox->itemData( ui->templateComboBox->currentIndex() );
		if( curProjVar.isValid() ) {
			const ProjectTemplate *curTmpl = reinterpret_cast<const ProjectTemplate *>( curProjVar.value<void*>() );
			oldTemplateId = curTmpl->getId();
		}
	}
	else { // set to OpenGL Basic by default
		oldTemplateId = "org.libcinder.apptemplates.basicopengl";
	}

	ui->templateComboBox->clear();

	mProjectTemplates.clear();
	
	// add all the built-in templates
	const QList<ProjectTemplate> &templates = ProjectTemplateManager::getTemplates();
	for( QList<ProjectTemplate>::ConstIterator tmplIt = templates.begin(); tmplIt != templates.end(); ++tmplIt ) {
		mProjectTemplates.push_back( *tmplIt );
		ui->templateComboBox->addItem( tmplIt->getName(), qVariantFromValue( (void*)(&mProjectTemplates.back() )) );
	}
	
	// add all the CinderBlock templates
	const QList<ProjectTemplate> &cblockTemplates = CinderBlockManager::getProjectTemplates();
	
	if( ! cblockTemplates.empty() )
		ui->templateComboBox->insertSeparator( ui->templateComboBox->count() );
	for( QList<ProjectTemplate>::ConstIterator tmplIt = cblockTemplates.begin(); tmplIt != cblockTemplates.end(); ++tmplIt ) {
		mProjectTemplates.push_back( *tmplIt );
		ui->templateComboBox->addItem( tmplIt->getName(), qVariantFromValue( (void*)(&mProjectTemplates.back() )) );
	}
	
	// if we have a template with the same ID as the old ID, select that
	for( QList<ProjectTemplate>::ConstIterator tmplIt = templates.begin(); tmplIt != templates.end(); ++tmplIt ) {
		if( ( ! oldTemplateId.isEmpty() ) && ( tmplIt->getId() == oldTemplateId ) ) {
			ui->templateComboBox->setCurrentIndex( std::distance( templates.begin(), tmplIt ) );
			break;
		}
	}

	// Make sure all blocks required by a given template are present
	for( QList<ProjectTemplate>::ConstIterator tmplIt = mProjectTemplates.begin(); tmplIt != mProjectTemplates.end(); ++tmplIt ) {
		const QList<QString> &dependencyNames = tmplIt->getRequires();
		for( QList<QString>::ConstIterator dependencyIt = dependencyNames.begin(); dependencyIt != dependencyNames.end(); ++dependencyIt ) {
			CinderBlock *depend = mParent->findCinderBlockById( *dependencyIt );
			if( ! depend )
				mParent->getTemplateErrorList().addError( QString( "Template '") + tmplIt->getName() + "' missing required block '" + *dependencyIt + "'" );
		}
	}

	if( mParent->getTemplateErrorList().empty() )
		ui->showErrorsButton->hide();
	else
		ui->showErrorsButton->show();

	ui->cinderVersionPathLabel->setText( QDir::toNativeSeparators( mParent->getCinderLocation() ) );
}

QString WizardPageMain::getTemplateValue() const
{
    return ui->templateComboBox->currentText();
}

QString WizardPageMain::getProjectName() const
{
    return ui->projectNameLineEdit->text();
}

QString WizardPageMain::getLocation() const
{
    return ui->locationLineEdit->text();
}

void WizardPageMain::enablePlatform( size_t platform, bool enable )
{
    if( enable )
        ui->compilerList->item( platform )->setFlags( ui->compilerList->item( platform )->flags() | (Qt::ItemIsEnabled | Qt::ItemIsSelectable) );
    else
        ui->compilerList->item( platform )->setFlags( ui->compilerList->item( platform )->flags() & (~(Qt::ItemIsEnabled | Qt::ItemIsSelectable)) );
}

bool WizardPageMain::isXcodeSelected() const
{
    return ui->compilerList->item( XCODE_INDEX )->isSelected();
}

bool WizardPageMain::isXcodeIosSelected() const
{
    return ui->compilerList->item( XCODE_IOS_INDEX )->isSelected();
}

bool WizardPageMain::isVc2012WinrtSelected() const
{
	return ui->compilerList->item( VC2012_WINRT_INDEX )->isSelected();
}

bool WizardPageMain::isVc2013Selected() const
{
	return ui->compilerList->item( VC2013_INDEX )->isSelected();
}

QString WizardPageMain::getCinderLocation() const
{
	return Preferences::getCinderVersions()[ui->cinderVersionComboBox->currentIndex()].path;
}

bool WizardPageMain::shouldCreateGitRepo() const
{
	return ui->sourceControlCheckBox->isChecked();
}

void WizardPageMain::closeEvent( QCloseEvent *event )
{
    QSettings settings;
    settings.setValue( "location", ui->locationLineEdit->text() );
    event->accept();
}

const ProjectTemplate& WizardPageMain::getProjectTemplate() const
{
	QVariant curProjVar = ui->templateComboBox->itemData( ui->templateComboBox->currentIndex() );
	const ProjectTemplate *curTmpl = reinterpret_cast<const ProjectTemplate *>( curProjVar.value<void*>() );
	return *curTmpl;
}

void WizardPageMain::updateCinderVersionsCtrl()
{
    // Clear
    ui->cinderVersionComboBox->clear();
    // Build
	QList<Preferences::CinderVersion>::const_iterator cit = Preferences::getCinderVersions().begin();
	for( ; cit != Preferences::getCinderVersions().end(); ++cit ) {
        ui->cinderVersionComboBox->addItem( cit->name );
		if( ! cit->valid )
			ui->cinderVersionComboBox->setItemData( ui->cinderVersionComboBox->count() - 1, QVariant(QColor(Qt::red)), Qt::TextColorRole);
    }

	if( ! Preferences::getCinderVersions()[ui->cinderVersionComboBox->currentIndex()].valid )
		ui->cinderVersionComboBox->setStyleSheet("QComboBox { background-color: red; }");
}

bool WizardPageMain::isComplete() const
{
	bool noneAreEmpty = ! ( ui->projectNameLineEdit->text().isEmpty() ||
							ui->locationLineEdit->text().isEmpty() );
	bool hasCompiler = false;
	for( size_t i = 0; i < NUM_PLATFORMS; ++i )
		if( ui->compilerList->item( i )->isSelected() )
			hasCompiler = true;
	
	return noneAreEmpty && hasCompiler;
}

void WizardPageMain::validateNextButton()
{
	emit QWizardPage::completeChanged();
}

void WizardPageMain::on_locationButton_clicked()
{
    QFileDialog dirSelDlg( this );
    dirSelDlg.setFileMode( QFileDialog::Directory );
    dirSelDlg.setOptions( QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly );
    dirSelDlg.setDirectory( ui->locationLineEdit->text() );
    if( dirSelDlg.exec() ) {
        // We should always have one item, if we don't. Something has gone horribly wrong.
        QStringList items = dirSelDlg.selectedFiles();
        ui->locationLineEdit->setText( QDir::toNativeSeparators( items[0] ) );
    }
}

void WizardPageMain::on_cinderVersionComboBox_activated( QString /*value*/ )
{
	if( ui->cinderVersionComboBox->currentIndex() < 0 )
		return;
	
	Preferences::CinderVersion cv = Preferences::getCinderVersions()[ui->cinderVersionComboBox->currentIndex()];
	ui->cinderVersionPathLabel->setText( QDir::toNativeSeparators( cv.path ) );
	mParent->setCinderLocation( QDir::toNativeSeparators( cv.path ) );

	if( ! Preferences::getCinderVersions()[ui->cinderVersionComboBox->currentIndex()].valid )
		ui->cinderVersionComboBox->setStyleSheet("QComboBox { background-color: red; }");
	else
		ui->cinderVersionComboBox->setStyleSheet(QString());
}

void WizardPageMain::updateProjectNameStatus()
{
	ui->projectNameLineEdit->setPalette( mLocationPaletteOrig );

	QFileInfo fi( QDir( ui->locationLineEdit->text() ).absoluteFilePath( ui->projectNameLineEdit->text() ) );
	if( fi.exists() ) {
		QPalette pal = ui->projectNameLineEdit->palette();
		pal.setColor( QPalette::Base, QColor( 0xFF, 0x63, 0x47 ) );
		ui->projectNameLineEdit->setPalette( pal );
	}
}

void WizardPageMain::on_projectNameLineEdit_textChanged( QString /*text*/ )
{
	validateNextButton();
	updateProjectNameStatus();
}

void WizardPageMain::on_locationLineEdit_textChanged( QString text )
{
	validateNextButton();
	updateProjectNameStatus();

    ui->locationLineEdit->setPalette( mLocationPaletteOrig );

    QFileInfo fi( text );
    if( ! ( fi.exists() && fi.isDir() ) ) {
        QPalette pal = ui->locationLineEdit->palette();
        pal.setColor( QPalette::Base, QColor( 0xFF, 0x63, 0x47 ) );
        ui->locationLineEdit->setPalette( pal );
    }
}

void WizardPageMain::on_compilerList_itemSelectionChanged()
{
	validateNextButton();
}

void WizardPageMain::on_templateComboBox_currentIndexChanged(int idx)
{
	if( idx == -1 || ( ui->templateComboBox->count() == 0 ) )
		return;
	
	int curIdx = ui->templateComboBox->currentIndex();
	QVariant curProjVar = ui->templateComboBox->itemData( curIdx );
	if( curProjVar.isValid() ) {
		const ProjectTemplate *curTmpl = reinterpret_cast<const ProjectTemplate *>( curProjVar.value<void*>() );
		for( size_t p = 0; p < NUM_PLATFORMS; ++p )
			enablePlatform( p, curTmpl->supportsConditions( mPlatformConditions[p] ) );	
	}
}

void WizardPageMain::on_showErrorsButton_clicked()
{
    ErrorListDialog dlg( this );
    dlg.show( mParent->getTemplateErrorList() );
}
