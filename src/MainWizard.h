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
