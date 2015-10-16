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

#ifndef PREFS_H
#define PREFS_H

#include <QDialog>
#include <QMap>
#include "TinderBox.h"

namespace Ui {
	class Prefs;
}

class Preferences {
  public:
	static int	addCinderVersion( const QString &name, const QString &path, bool shouldSave ) { return get()->addCinderVersionInst( name, path, shouldSave ); }
	static void removeCinderVersion( size_t index ) { get()->removeCinderVersionInst( index ); }
	static void updateCinderVersion( size_t index, const QString &name, const QString &path ) { get()->updateCinderVersionInst( index, name, path ); }

	struct CinderVersion {
		QString name;
		QString path;
		bool	valid; // appears to be an actual Cinder repo
		bool	shouldSave; // if we added the housing Cinder repo without user request, don't serialize
	};
	static QList<CinderVersion>&		getCinderVersions() { return get()->mCinderVersions; }
	static const CinderVersion&			getDefaultCinderVersion() { return *(get()->mCinderVersions.begin()); }

	static QString		getOutputPath() { return get()->mOutputPath; }
	static void			setOutputPath( const QString &outputPath ) { get()->mOutputPath = outputPath; get()->save(); }

	static bool				isGitAvailable() { return true; }
	static QString			getGitPath() { return QString::fromUtf8( "git" ); }
	
	static bool				getCreateGitRepoDefault() { return get()->mCreateGitRepoDefault; }
	static void				setCreateGitRepoDefault( bool create ) { get()->mCreateGitRepoDefault = create; get()->save(); }

  private:
	Preferences() {}
	static Preferences*	get();

	void		load();
	void		save();
	int			addCinderVersionInst( const QString &name, const QString &path, bool shouldSave );
	void		removeCinderVersionInst( size_t index );
	void		updateCinderVersionInst( size_t index, const QString &name, const QString &path );

	QList<CinderVersion>		mCinderVersions;
	QString						mOutputPath;
	bool						mCreateGitRepoDefault;
};

class QTreeWidgetItem;
class Prefs : public QDialog
{
    Q_OBJECT

public:
	explicit Prefs(QWidget *parent = 0);
    ~Prefs();

private:    
    Ui::Prefs *ui;

	QList<QPair<QString, Preferences::CinderVersion> >       mCinderVersions;

    void    loadCinderVersionsFromPrefs();
    void    saveCinderVersionsToPrefs();
    void    updateCinderVersionsCtrl();

private slots:
    void on_removeButton_clicked();
    void on_addButton_clicked();
	void on_cinderVersions_itemChanged(QTreeWidgetItem *item, int column);
};

#endif // PREFS_H
