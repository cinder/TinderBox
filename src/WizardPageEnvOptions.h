#ifndef WIZARDPAGEENVOPTIONS_H
#define WIZARDPAGEENVOPTIONS_H

#include <QWizardPage>

namespace Ui {
class WizardPageEnvOptions;
}

class WizardPageEnvOptions : public QWizardPage
{
    Q_OBJECT

public:
    explicit WizardPageEnvOptions(QWidget *parent = 0);
    ~WizardPageEnvOptions();

	bool	isWin32Selected() const;
	bool	isX64Selected() const;
	bool	isDesktopGlSelected() const;
	bool	isAngleSelected() const;

public slots:
	void	updateNextButton( bool /*ignored*/ );

private:
    Ui::WizardPageEnvOptions *ui;
};

#endif // WIZARDPAGEENVOPTIONS_H
