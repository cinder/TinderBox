#include "Preferences.h"
#include "ui_PrefsDlg.h"

#include <QFileDialog>
#include <QSettings>

// Saves to ~/Library/Preferences/org.libcinder.TinderBox.plist on Mac OS X

////////////////////////////////////////////////////////////////////////////////////////////
// Preferences
Preferences* Preferences::get()
{
	static Preferences *inst = 0;
	if( ! inst ) {
		inst = new Preferences();
		inst->load();
	}

	return inst;
}

void Preferences::load()
{
	mCinderVersions.clear();

	QSettings settings;
	int size = settings.beginReadArray( "cinderVersions" );
	for( int i = 0; i < size; ++i ) {
		settings.setArrayIndex( i );
		CinderVersion cv;
		cv.name = settings.value( "cinderVersionName" ).toString();
		cv.path = settings.value( "cinderVersionPath" ).toString();
		cv.valid = QDir( cv.path + "/blocks/__appTemplates" ).exists();
		mCinderVersions.push_back( cv );
	}
	settings.endArray();
	mOutputPath = settings.value( "outputPath", "" ).toString();
	mCreateGitRepoDefault = settings.value( "createGitRepoDefault", QVariant( true ) ).toBool();
}

void Preferences::save()
{
	QSettings settings;

	int i = 0;
	QList<CinderVersion>::const_iterator cit = mCinderVersions.begin();
	settings.beginWriteArray( "cinderVersions" );
	for( ; cit != mCinderVersions.end(); ++cit, ++i ) {
		settings.setArrayIndex( i );
		settings.setValue( "cinderVersionName", cit->name );
		settings.setValue( "cinderVersionPath", cit->path );
	}
	settings.endArray();
	settings.setValue( "outputPath", mOutputPath );
	settings.setValue( "createGitRepoDefault", mCreateGitRepoDefault );
	settings.sync();
}

void Preferences::addCinderVersionInst( const QString &name, const QString &path )
{
	CinderVersion v;
	v.name = name;
	v.path = path;
	mCinderVersions.push_back( v );

	save();
}

void Preferences::removeCinderVersionInst( size_t index )
{
	mCinderVersions.erase( mCinderVersions.begin() + index );

	save();
}

void Preferences::updateCinderVersionInst( size_t index, const QString &name, const QString &path )
{
	Preferences::CinderVersion &cv( getCinderVersions()[index] );
	cv.name = name;
	cv.path = path;

	save();
}

////////////////////////////////////////////////////////////////////////////////////////////
// Preferences
Prefs::Prefs(QWidget *parent)
: QDialog( parent, Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint ),
  ui( new Ui::Prefs )
{
    ui->setupUi(this);

    updateCinderVersionsCtrl();
}

Prefs::~Prefs()
{
    delete ui;
}

void Prefs::updateCinderVersionsCtrl()
{
    // Clear
    ui->cinderVersions->clear();
    // Build
	QList<Preferences::CinderVersion>::const_iterator cit = Preferences::getCinderVersions().begin();
	for( ; cit != Preferences::getCinderVersions().end(); ++cit ) {
        QTreeWidgetItem *item = new QTreeWidgetItem( QStringList( cit->name ) );
        item->setText( 1, cit->path );
        item->setFlags( item->flags() | Qt::ItemIsEditable );
        ui->cinderVersions->addTopLevelItem( item );
    }
    
    ui->removeButton->setEnabled( Preferences::getCinderVersions().size() > 1 );
}

void Prefs::on_addButton_clicked()
{
	QFileDialog dirSelDlg( this );
	dirSelDlg.setFileMode( QFileDialog::Directory );
	dirSelDlg.setOptions( QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly );
	if( dirSelDlg.exec() ) {
		// We should always have one item, if we don't. Something has gone horribly wrong.
		QStringList items = dirSelDlg.selectedFiles();
		QDir dir( items[0] );
		Preferences::addCinderVersion( dir.dirName(), items[0] );

		updateCinderVersionsCtrl();
	}
}

void Prefs::on_removeButton_clicked()
{
    QTreeWidgetItem *selItem = ui->cinderVersions->currentItem();
    if( selItem == NULL )
		return;
    Preferences::removeCinderVersion( ui->cinderVersions->indexOfTopLevelItem( selItem ) );

    updateCinderVersionsCtrl();
}

void Prefs::on_cinderVersions_itemChanged(QTreeWidgetItem *item, int /*column*/ )
{
	Preferences::updateCinderVersion( ui->cinderVersions->indexOfTopLevelItem( item ), item->text( 0 ), item->text( 1 ) );
}
