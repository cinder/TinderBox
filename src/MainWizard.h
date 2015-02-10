#ifndef MAINWIZARD_H
#define MAINWIZARD_H

#include <QWizard>
#include "Preferences.h"
#include "CinderBlock.h"
#include "ProjectTemplate.h"
#include "ErrorList.h"

class MainWizard : public QWizard
{
	Q_OBJECT
public:
	explicit MainWizard(QWidget *parent = 0);
void paintEvent( QPaintEvent *event );
	virtual QSize	sizeHint() const { return QSize( 700, 500 ); }

	const QString&	getCinderLocation() const { return mCinderLocation; }
	void			setCinderLocation( const QString &path, bool updateGui = true );
	QList<CinderBlock>&		getCinderBlocks() { return mCinderBlocks; }
	CinderBlock*			findCinderBlockById( const QString &id );
	void					refreshRequiredBlocks();

	const QString&	getDefaultLocation() const { return mDefaultLocation; }

	bool				shouldCreateGitRepo() const { return mShouldCreateGitRepo; }
	const ErrorList&	getTemplateErrorList() const { return mTemplateErrors; }
	ErrorList&			getTemplateErrorList() { return mTemplateErrors; }
	const ErrorList&	getCinderBlockErrorList() const { return mCinderBlockErrors; }	
signals:
	
public slots:
	void		preferencesClicked();
	void		advancingToNextPage( int newId );
	void		generateProject();

private:
    int             nextId() const override;
	void			checkForFirstTime();
	void			loadPreferences();
	void			loadTemplates();
	void			requireBlocks( const QList<QString> &dependencyNames );

	class WizardPageMain			*mWizardPageMain;
	class WizardPageCinderBlocks	*mWizardPageCinderBlocks;
    class WizardPageEnvOptions      *mWizardPageEnvOptions;
	Prefs					*mPrefs;
	QString					mCinderLocation;
	ErrorList				mTemplateErrors, mCinderBlockErrors;
	QString					mDefaultLocation;
	bool					mShouldCreateGitRepo;

	QList<CinderBlock>		mCinderBlocks;
};

#endif // MAINWIZARD_H
