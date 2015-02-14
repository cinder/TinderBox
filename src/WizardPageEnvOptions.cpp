#include "WizardPageEnvOptions.h"
#include "ui_WizardPageEnvOptions.h"

WizardPageEnvOptions::WizardPageEnvOptions(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::WizardPageEnvOptions)
{
    ui->setupUi(this);

	connect( ui->vc2013Win32CheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );
	connect( ui->vc2013X64CheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );
	connect( ui->vc2013OpenGlCheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );
	connect( ui->vc2013AngleCheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );

	QFont font = ui->envOptionsLabel->font();
	font.setBold( true ); font.setPointSize( 12 );
	ui->envOptionsLabel->setFont( font );
	font.setPointSize( 10 );
	ui->vc2013HeaderLabel->setFont( font );
	font.setPointSize( 9 );
	ui->vc2013PlatformLabel->setFont( font );
	ui->vc2013RendererLabel->setFont( font );
}

bool WizardPageEnvOptions::isWin32Selected() const
{
	return ui->vc2013Win32CheckBox->isChecked();
}

bool WizardPageEnvOptions::isX64Selected() const
{
	return ui->vc2013X64CheckBox->isChecked();
}

bool WizardPageEnvOptions::isDesktopGlSelected() const
{
	return ui->vc2013OpenGlCheckBox->isChecked();
}

bool WizardPageEnvOptions::isAngleSelected() const
{
	return ui->vc2013AngleCheckBox->isChecked();
}

void WizardPageEnvOptions::updateNextButton( bool /*ignored*/ )
{
	bool platformChecked = isWin32Selected() || isX64Selected();
	bool rendererChecked = isDesktopGlSelected() || ( isWin32Selected() && isAngleSelected() );
	wizard()->button( QWizard::NextButton )->setEnabled( platformChecked && rendererChecked );
}

WizardPageEnvOptions::~WizardPageEnvOptions()
{
    delete ui;
}
