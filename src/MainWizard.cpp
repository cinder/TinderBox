#include "MainWizard.h"
#include "WizardPageMain.h"
#include "WizardPageCinderBlocks.h"
#include "WizardPageEnvOptions.h"
#include "Preferences.h"
#include "FirstTimeDlg.h"
#include "Instancer.h"
#include "ProjectTemplateManager.h"
#include "CinderBlockManager.h"
#include "GeneratorXcodeMac.h"
#include "GeneratorXcodeIos.h"
#include "GeneratorVc2013Winrt.h"
#include "GeneratorVc2013.h"
#include "Util.h"

#include <QAbstractButton>
#include <QFileDialog>
#include <QUuid>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QPainter>
#include <QAction>
#include <QMenuBar>
#include <QMainWindow>
#include <QCoreApplication>

enum { PAGE_MAIN = 0, PAGE_ENV_OPTIONS = 1, PAGE_CINDER_BLOCKS = 2 };

MainWizard::MainWizard(QWidget *parent) :
	QWizard(parent),
	mWizardPageMain( 0 ), mWizardPageCinderBlocks( 0 ) // NULL to ensure we don't update them prematurely
{
	checkForFirstTime();
	loadPreferences();
	setCinderPathToHousingPath();

	loadTemplates();

	setWizardStyle( QWizard::ClassicStyle );
	setOption(QWizard::IgnoreSubTitles, true);
	setOption( QWizard::HaveCustomButton1, true );
	setOption( QWizard::NoBackButtonOnStartPage, true );
	setButtonText( QWizard::CustomButton1, "Settings..." );
	QList<QWizard::WizardButton> buttonLayout;
	buttonLayout << QWizard::Stretch << QWizard::CustomButton1 << QWizard::BackButton << QWizard::NextButton << QWizard::FinishButton;
	setButtonLayout( buttonLayout );

	mWizardPageMain = new WizardPageMain( this );
    mWizardPageEnvOptions = new WizardPageEnvOptions( this );
	mWizardPageCinderBlocks = new WizardPageCinderBlocks( this );

    addPage( mWizardPageMain );
    addPage( mWizardPageEnvOptions );
    addPage( mWizardPageCinderBlocks );

	connect( this, SIGNAL(customButtonClicked(int)), this, SLOT(preferencesClicked()) );
	connect( this, SIGNAL(currentIdChanged(int)), this, SLOT(advancingToNextPage(int)) );
	connect( this, SIGNAL(accepted()), this, SLOT(generateProject()) );

	setCinderLocationByIndex( mCinderLocationIndex );

	mPrefs = new Prefs( this );
}

void MainWizard::paintEvent( QPaintEvent */*event*/ )
{
	QPainter painter( this );
	QPixmap pmap(":/resources/background.png");
	painter.setRenderHint( QPainter::SmoothPixmapTransform );
	painter.setOpacity( 0.1f );	
#if defined Q_OS_WIN
	float scale = 0.5f;
	int offset = 6;
#elif defined Q_OS_MAC
	float scale = 0.7f;
	int offset = 10;
#endif
	painter.drawPixmap( offset, height() - pmap.height() * scale - offset, pmap.width() * scale, pmap.height() * scale, pmap );
}

void MainWizard::loadPreferences()
{
	mDefaultLocation = Preferences::getOutputPath();
	if( mDefaultLocation.isEmpty() ) {
#if defined Q_OS_WIN
		mDefaultLocation = QDir::toNativeSeparators( joinPath( QDir::homePath(), "Documents" ) );
#elif defined Q_OS_MAC
		mDefaultLocation = QDir::toNativeSeparators( joinPath( QDir::homePath(), "Documents" ) );
#endif
	}
}

void MainWizard::loadTemplates()
{
	// Templates.xml
	try {
		ProjectTemplateManager::inst(); // force the manager to start up
	}
	catch( const TinderBoxExc &e ) {
		showErrorMsg( e.msg(), "Templates.xml Error" );

		// There is a probably a better way to do this. But for now
		// we'll call the system level exit.
		exit( 0 );
	}
}

int MainWizard::nextId() const
{
    // If we're coming from the first page and the user has enabled VC2013, we need to present some options
	if( currentId() == PAGE_MAIN ) {
		if( mWizardPageMain->isVc2013Selected() || mWizardPageMain->isVc2013WinrtSelected() )
			return PAGE_ENV_OPTIONS;
        else
			return PAGE_CINDER_BLOCKS;
    }
	else if( currentId() == PAGE_CINDER_BLOCKS ) // last page; we're done
        return -1;
    else
		return PAGE_CINDER_BLOCKS;
}

// Returns the path to the version of Cinder which the application is being run from
QString MainWizard::getHousingCinderPath()
{
	for( int i = 1; i < 8; ++i ) {
		QString test = QCoreApplication::applicationDirPath();
		for( int up = 0; up < i; ++up )
			test += "/..";
		if( QFileInfo( test + "/blocks/__AppTemplates" ).isDir() ) {
			return QFileInfo( test ).absoluteFilePath();
		}
	}

	return QString();
}

void MainWizard::setCinderPathToHousingPath()
{
	QString housingPath = getHousingCinderPath();

	if( housingPath.length() > 0 ) {
		// make sure this is already a known Cinder path
		int index = Preferences::addCinderVersion( QDir( housingPath ).dirName(), QDir( housingPath ).absolutePath(), false );
		mCinderLocationIndex = index;
	}
	else
		mCinderLocationIndex = 0;
}

const QString& MainWizard::getCinderLocation() const
{
	return Preferences::getCinderVersions()[mCinderLocationIndex].path;
}

// Returns 'true' if this was the first time the user has run TinderBox
bool MainWizard::checkForFirstTime()
{
	// if there are no Cinder versions, try to add the one we're in and we won't bug the user.
	if( Preferences::getCinderVersions().isEmpty() ) {
		QString housingPath = getHousingCinderPath();
		if( housingPath.length() > 0 ) {
			Preferences::addCinderVersion( QDir( housingPath ).dirName(), QDir( housingPath ).absolutePath(), false );
			Preferences::setOutputPath( QDir::homePath() );
			return true;
		}

		FirstTimeDlg firstTimeDlg( this );
		firstTimeDlg.exec();

		QFileDialog dirSelDlg( this );
		dirSelDlg.setFileMode( QFileDialog::Directory );
		dirSelDlg.setOptions( QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly );
		if( dirSelDlg.exec() ) {
			QDir cinderPath( dirSelDlg.selectedFiles()[0] );
			Preferences::addCinderVersion( cinderPath.dirName(), cinderPath.absolutePath(), true );
			Preferences::setOutputPath( QDir::homePath() );
		}
		else {
			exit( 0 );
		}

		return true;
	}

	return false;
}

void MainWizard::generateProject()
{
	try {
		Instancer gen( mWizardPageMain->getProjectTemplate() );
#if 0
		gen.setProjectName( mWizardPageMain->getProjectName() + QUuid::createUuid().toString() );
#else
		gen.setProjectName( mWizardPageMain->getProjectName() );
#endif
		gen.setNamePrefix( mWizardPageMain->getProjectName() );
		gen.setBaseLocation( mWizardPageMain->getLocation() );
		gen.setCinderAbsolutePath( mWizardPageMain->getCinderLocation() );

		if( mWizardPageMain->isXcodeSelected() )
			gen.addGenerator( new GeneratorXcodeMac() );
		if( mWizardPageMain->isXcodeIosSelected() )
			gen.addGenerator( new GeneratorXcodeIos() );
		if( mWizardPageMain->isVc2013Selected() ) {
			GeneratorVc2013::Options options;
			options.enableWin32( mWizardPageEnvOptions->isVc2013Win32Selected() );
			options.enableX64( mWizardPageEnvOptions->isVc2013X64Selected() );
			options.enableDesktopGl( mWizardPageEnvOptions->isVc2013DesktopGlSelected() );
			options.enableAngle( mWizardPageEnvOptions->isVc2013AngleSelected() );
			gen.addGenerator( new GeneratorVc2013( options ) );
		}
		if( mWizardPageMain->isVc2013WinrtSelected() ) {
			GeneratorVc2013WinRt::Options options;
			options.enableWin32( mWizardPageEnvOptions->isVc2013WinRtWin32Selected() );
			options.enableX64( mWizardPageEnvOptions->isVc2013WinRtX64Selected() );
			options.enableArm( mWizardPageEnvOptions->isVc2013WinRtArmSelected() );
			gen.addGenerator( new GeneratorVc2013WinRt( options ) );
		}
		for( QList<CinderBlock>::ConstIterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
			if( blockIt->getInstallType() != CinderBlock::INSTALL_NONE )
				gen.addCinderBlock( *blockIt );
		}

		gen.generate( mWizardPageMain->shouldCreateGitRepo() );

		QString localPath = joinPath( gen.getBaseLocation(), gen.getProjectName() );
#if defined Q_OS_MACX
		localPath = joinPath( localPath, "xcode" );
#endif
		QUrl localUrl = QUrl::fromLocalFile( QDir::toNativeSeparators( localPath ) );
		QDesktopServices::openUrl( localUrl );

		Preferences::setOutputPath( mWizardPageMain->getLocation() );
		Preferences::setCreateGitRepoDefault( mWizardPageMain->shouldCreateGitRepo() );
	}
	catch( const TinderBoxExc &e ) {
		showErrorMsg( e.msg() );
	}
}

void MainWizard::setCinderLocationByIndex( int index )
{
	mCinderLocationIndex = index;

	auto cinderLocationPath = Preferences::getCinderVersions()[index].path;

	ProjectTemplateManager::clear();
	mTemplateErrors.clear();
	ProjectTemplateManager::setCinderDir( cinderLocationPath, &mTemplateErrors );

	CinderBlockManager::clear();
	mCinderBlockErrors.clear();
	CinderBlockManager::scan( cinderLocationPath, &mCinderBlockErrors );

	mCinderBlocks = CinderBlockManager::getCinderBlocks();

	if( mWizardPageMain )
		mWizardPageMain->setCinderLocationByIndex( index );
	if( mWizardPageCinderBlocks )
		mWizardPageCinderBlocks->setCinderLocation( cinderLocationPath );
}

CinderBlock* MainWizard::findCinderBlockById( const QString &searchId )
{
	for( QList<CinderBlock>::Iterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
		QString thisBlockId = blockIt->getId();
		if( thisBlockId.compare( searchId, Qt::CaseInsensitive ) == 0 )
			return &(*blockIt);
	}

	return NULL;
}

void MainWizard::requireBlocks( const QList<QString> &dependencyNames )
{
	for( QList<QString>::ConstIterator dependencyIt = dependencyNames.begin(); dependencyIt != dependencyNames.end(); ++dependencyIt ) {
		CinderBlock *depend = findCinderBlockById( *dependencyIt );
		if( depend && depend->getInstallType() == CinderBlock::INSTALL_NONE ) {
			depend->setInstallType( depend->isCore() ? CinderBlock::INSTALL_REFERENCE : CinderBlock::INSTALL_COPY );
			depend->setRequired( true );
		}
	}
}

void MainWizard::refreshRequiredBlocks()
{
	// clear out the requireds
	for( QList<CinderBlock>::Iterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt )
		blockIt->setRequired( false );

	// walk the dependencies of this template
	requireBlocks( mWizardPageMain->getProjectTemplate().getRequires() );

	// walk the dependencies of installed CinderBlocks
	for( QList<CinderBlock>::ConstIterator blockIt = mCinderBlocks.begin(); blockIt != mCinderBlocks.end(); ++blockIt ) {
		if( blockIt->getInstallType() == CinderBlock::INSTALL_NONE )
			continue;
		else
			requireBlocks( blockIt->getRequires() );
	}
}

void MainWizard::advancingToNextPage( int newId )
{
	if( newId == PAGE_CINDER_BLOCKS ) { // update the cinderblock list if we're about to show it
		mShouldCreateGitRepo = mWizardPageMain->shouldCreateGitRepo(); // do this first
		refreshRequiredBlocks();
		mWizardPageCinderBlocks->setCinderLocation( mWizardPageMain->getCinderLocation() );
	}
}

void MainWizard::preferencesClicked()
{
	mPrefs->exec();
	mWizardPageMain->updateCinderVersionsCtrl();
}
