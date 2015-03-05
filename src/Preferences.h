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
