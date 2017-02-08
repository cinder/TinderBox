/*
 Copyright (c) 2015, The Cinder Project, All rights reserved.

 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and
    the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
    the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "Preferences.h"
#include "ui_PrefsDlg.h"

#include <QFileDialog>
#include <QSettings>

// Saves to ~/Library/Preferences/org.libcinder.TinderBox.plist on Mac OS X
// On modern OS X you need to precede with:
// killall -u andrewfb cfprefsd

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
		cv.shouldSave = true;
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
	for( ; cit != mCinderVersions.end(); ++cit ) {
		if( cit->shouldSave ) {
			settings.setArrayIndex( i++ );
			settings.setValue( "cinderVersionName", cit->name );
			settings.setValue( "cinderVersionPath", cit->path );
		}
	}
	settings.endArray();
	settings.setValue( "outputPath", mOutputPath );
	settings.setValue( "createGitRepoDefault", mCreateGitRepoDefault );
	settings.sync();
}

// returns the index if it already exists
int Preferences::addCinderVersionInst( const QString &name, const QString &path, bool shouldSave )
{
	// see if we already have this version
	for( int idx = 0; idx < mCinderVersions.size(); ++idx ) {
		if( QFileInfo( path ).absoluteFilePath() == QFileInfo( mCinderVersions[idx].path ).absoluteFilePath() )
			return idx;
	}

	CinderVersion v;
	v.name = name;
	v.path = path;
	v.valid = QDir( v.path + "/blocks/__appTemplates" ).exists();
	v.shouldSave = shouldSave;
	mCinderVersions.push_back( v );

	save();
	return mCinderVersions.size() - 1;
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
		Preferences::addCinderVersion( dir.dirName(), items[0], true );

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
