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
#include "GeneratorVc2012Winrt.h"
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

MainWizard::MainWizard(QWidget *parent) :
	QWizard(parent),
	mWizardPageMain( 0 ), mWizardPageCinderBlocks( 0 ) // NULL to ensure we don't update them prematurely
{
	checkForFirstTime();
	loadPreferences();
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

	setCinderLocation( Preferences::getDefaultCinderVersion().path, false );
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
    if( currentId() == 0 ) {
        if( mWizardPageMain->isVc2013Selected() )
            return 1;
        else
            return 2;
    }
    else if( currentId() == 2 ) // last page; we're done
        return -1;
    else
        return 2;
}

void MainWizard::checkForFirstTime()
{
	// if there are no Cinder versions, try to add the one we're in
	if( Preferences::getCinderVersions().isEmpty() ) {
		for( int i = 1; i < 8; ++i ) {
			QString test = QCoreApplication::applicationDirPath();
			for( int up = 0; up < i; ++up )
				test += "/..";
			if( QDir( test + "/blocks/__AppTemplates" ).exists() ) {
				Preferences::addCinderVersion( "HEAD", QDir( test ).absolutePath() );
				Preferences::setOutputPath( QDir::homePath() );
				break;
			}
		}
	}
	
	// Now check to see if we need to run the 'first time' dialog.
	if( Preferences::getCinderVersions().isEmpty() ){
		FirstTimeDlg firstTimeDlg( this );
		firstTimeDlg.exec();

		QFileDialog dirSelDlg( this );
		dirSelDlg.setFileMode( QFileDialog::Directory );
		dirSelDlg.setOptions( QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly );
		if( dirSelDlg.exec() ) {
			Preferences::addCinderVersion( "HEAD", dirSelDlg.selectedFiles()[0] );
			Preferences::setOutputPath( QDir::homePath() );
		}
		else {
			// User hit cancel - bail!

			// There is a probably a better way to do this. But for now
			// we'll call the system level exit.
			exit( 0 );
		}
	}
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
		if( mWizardPageMain->isVc2012WinrtSelected() )
			gen.addGenerator( new GeneratorVc2012Winrt() );
		if( mWizardPageMain->isVc2013Selected() )
			gen.addGenerator( new GeneratorVc2013() );

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

void MainWizard::setCinderLocation( const QString &path, bool updateGui )
{
	mCinderLocation = path;

	ProjectTemplateManager::clear();
	mTemplateErrors.clear();
	ProjectTemplateManager::setCinderDir( mCinderLocation, &mTemplateErrors );

	CinderBlockManager::clear();
	mCinderBlockErrors.clear();
	CinderBlockManager::scan( mCinderLocation, &mCinderBlockErrors );

	mCinderBlocks = CinderBlockManager::getCinderBlocks();

	if( updateGui ) {
		if( mWizardPageMain )
			mWizardPageMain->updateTemplates();
		if( mWizardPageCinderBlocks )
			mWizardPageCinderBlocks->setCinderLocation( path );
	}
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
    if( newId == 2 ) { // update the cinderblock list if we're about to show it
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
