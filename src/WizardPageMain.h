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

#ifndef WizardPageMain_H
#define WizardPageMain_H

#include <QWizardPage>

#include "TinderBox.h"
#include "Preferences.h"
#include "ProjectTemplate.h"

class MainWizard;

namespace Ui {
class WizardPageMain;
}

class WizardPageMain : public QWizardPage
{
	Q_OBJECT
	
public:
	explicit WizardPageMain( MainWizard *parent = 0);
	~WizardPageMain();

	const ProjectTemplate&	getProjectTemplate() const;
	QString					getProjectName() const;
	QString					getLocation() const;
	QString					getCinderLocation() const;
	void					setCinderLocationByIndex( int index );
	bool					shouldCreateGitRepo() const;

	void	enablePlatform( size_t platform, bool enable );
	bool	isXcodeSelected() const;
	bool	isXcodeIosSelected() const;
    bool	isVc2015WinrtSelected() const;
	bool	isVc2013Selected() const;
    bool	isLinuxCmakeSelected() const;

	void                updateCinderVersionsCtrl();
	void				updateTemplates();
	virtual bool		isComplete() const;
	virtual void		initializePage();
protected:
    virtual void        closeEvent( QCloseEvent *event );
	
private:
	Ui::WizardPageMain	*ui;
	MainWizard		*mParent;
	bool			mInitialized;

	QList<ProjectTemplate>					mProjectTemplates;
    QPalette                                mLocationPaletteOrig;
    QList<QMap<QString,QString> >			mPlatformConditions;

	QString				getTemplateValue() const;
	void				updateProjectNameStatus();
	void                validateNextButton();

private slots:
    void on_compilerList_itemSelectionChanged();
    void on_locationLineEdit_textChanged( QString text );
    void on_projectNameLineEdit_textChanged( QString text );
	void on_cinderVersionComboBox_activated( QString value );
    void on_locationButton_clicked();
	void on_templateComboBox_currentIndexChanged(int index);
	void on_showErrorsButton_clicked();
};

#endif // WizardPageMain_H
