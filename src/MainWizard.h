#ifndef MAINWIZARD_H
#define MAINWIZARD_H

#include <QWizard>
#include "Preferences.h"
#include "CinderBlock.h"
#include "ProjectTemplate.h"
#include "ErrorList.h"

class WizardPageMain;
class WizardPageEnvOptions;
class WizardPageCinderBlocks;

class MainWizard : public QWizard
{
	Q_OBJECT
public:
	explicit MainWizard(QWidget *parent = 0);
void paintEvent( QPaintEvent *event );
	virtual QSize	sizeHint() const { return QSize( 700, 500 ); }

	QString			getHousingCinderPath();
	void			setCinderPathToHousingPath();
	const QString&	getCinderLocation() const;
	void			setCinderLocationByIndex( int index );
	QList<CinderBlock>&		getCinderBlocks() { return mCinderBlocks; }
	CinderBlock*			findCinderBlockById( const QString &id );
	void					refreshRequiredBlocks();

	const QString&	getDefaultLocation() const { return mDefaultLocation; }

	bool				shouldCreateGitRepo() const { return mShouldCreateGitRepo; }
	const ErrorList&	getTemplateErrorList() const { return mTemplateErrors; }
	ErrorList&			getTemplateErrorList() { return mTemplateErrors; }
	const ErrorList&	getCinderBlockErrorList() const { return mCinderBlockErrors; }	

	const WizardPageMain*	getWizardPageMain() const { return mWizardPageMain; }
signals:
	
public slots:
	void		preferencesClicked();
	void		advancingToNextPage( int newId );
	void		generateProject();

private:
    int             nextId() const override;
	bool			checkForFirstTime();
	void			loadPreferences();
	void			loadTemplates();
	void			requireBlocks( const QList<QString> &dependencyNames );

	WizardPageMain			*mWizardPageMain;
	WizardPageCinderBlocks	*mWizardPageCinderBlocks;
	WizardPageEnvOptions    *mWizardPageEnvOptions;
	Prefs					*mPrefs;
	int						mCinderLocationIndex;
	ErrorList				mTemplateErrors, mCinderBlockErrors;
	QString					mDefaultLocation;
	bool					mShouldCreateGitRepo;

	QList<CinderBlock>		mCinderBlocks;
};

#endif // MAINWIZARD_H
