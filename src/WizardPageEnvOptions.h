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

#ifndef WIZARDPAGEENVOPTIONS_H
#define WIZARDPAGEENVOPTIONS_H

#include <QWizardPage>
#include "MainWizard.h"

namespace Ui {
class WizardPageEnvOptions;
}

class WizardPageEnvOptions : public QWizardPage
{
    Q_OBJECT

public:
	explicit WizardPageEnvOptions( MainWizard *parent = 0 );
    ~WizardPageEnvOptions();

	void	initializePage() override;

	// VC 2015
	bool	isVc2015Win32Selected() const;
	bool	isVc2015X64Selected() const;
	bool	isVc2015DesktopGlSelected() const;
	bool	isVc2015AngleSelected() const;

	// VC 2015 WinRT
	bool	isVc2015WinRtWin32Selected() const;
	bool	isVc2015WinRtX64Selected() const;
	bool	isVc2015WinRtArmSelected() const;

public slots:
	void	updateNextButton( bool /*ignored*/ );

private:
	void	recursiveEnable( QLayout *layout, bool enable );

	Ui::WizardPageEnvOptions	*ui;
	MainWizard					*mParent;

	bool			mVc2015Enabled, mVc2015WinRtEnabled;
};

#endif // WIZARDPAGEENVOPTIONS_H
