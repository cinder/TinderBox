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

	// VC 2013
	bool	isVc2013Win32Selected() const;
	bool	isVc2013X64Selected() const;
	bool	isVc2013DesktopGlSelected() const;
	bool	isVc2013AngleSelected() const;

	// VC 2013 WinRT
	bool	isVc2013WinRtWin32Selected() const;
	bool	isVc2013WinRtX64Selected() const;
	bool	isVc2013WinRtArmSelected() const;

public slots:
	void	updateNextButton( bool /*ignored*/ );

private:
	void	recursiveEnable( QLayout *layout, bool enable );

	Ui::WizardPageEnvOptions	*ui;
	MainWizard					*mParent;

	bool			mVc2013Enabled, mVc2013WinRtEnabled;
};

#endif // WIZARDPAGEENVOPTIONS_H
