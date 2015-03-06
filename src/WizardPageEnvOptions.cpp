#include "WizardPageEnvOptions.h"
#include "ui_WizardPageEnvOptions.h"
#include "WizardPageMain.h"

WizardPageEnvOptions::WizardPageEnvOptions( MainWizard *parent ) :
	QWizardPage( parent ), mParent( parent ),
    ui(new Ui::WizardPageEnvOptions)
{
    ui->setupUi(this);

	// VC 2013
	connect( ui->vc2013Win32CheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );
	connect( ui->vc2013X64CheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );
	connect( ui->vc2013OpenGlCheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );
	connect( ui->vc2013AngleCheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );

	// VC 2013 WinRT
	connect( ui->vc2013WinRtWin32CheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );
	connect( ui->vc2013WinRtX64CheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );
	connect( ui->vc2013WinRtArmCheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );

#if ! defined( Q_OS_MAC )
	QFont font = ui->envOptionsLabel->font();
	font.setBold( true ); font.setPointSize( 12 );
	ui->envOptionsLabel->setFont( font );
	font.setPointSize( 10 );
	ui->vc2013HeaderLabel->setFont( font );
	font.setPointSize( 9 );
	ui->vc2013PlatformLabel->setFont( font );
	ui->vc2013RendererLabel->setFont( font );
#endif
}

void WizardPageEnvOptions::initializePage()
{
	mVc2013Enabled = mParent->getWizardPageMain()->isVc2013Selected();
	mVc2013WinRtEnabled = mParent->getWizardPageMain()->isVc2013WinrtSelected();

	recursiveEnable( ui->vc2013Layout, mVc2013Enabled );
	recursiveEnable( ui->vc2013WinRtLayout, mVc2013WinRtEnabled );
}

void WizardPageEnvOptions::recursiveEnable( QLayout *layout, bool enable )
{
	int numChildren = layout->count();
	for( int c = 0; c < numChildren; ++c ) {
		QLayoutItem *item = layout->itemAt( c );
		if( item->layout() )
			recursiveEnable( item->layout(), enable );
		else if( item->widget() )
			item->widget()->setEnabled( enable );
	}
}

bool WizardPageEnvOptions::isVc2013Win32Selected() const
{
	return ui->vc2013Win32CheckBox->isChecked();
}

bool WizardPageEnvOptions::isVc2013X64Selected() const
{
	return ui->vc2013X64CheckBox->isChecked();
}

bool WizardPageEnvOptions::isVc2013DesktopGlSelected() const
{
	return ui->vc2013OpenGlCheckBox->isChecked();
}

bool WizardPageEnvOptions::isVc2013AngleSelected() const
{
	return ui->vc2013AngleCheckBox->isChecked();
}

/////////////////////////////////////////////////////////////
// VC 2013 WinRT
bool WizardPageEnvOptions::isVc2013WinRtWin32Selected() const
{
	return ui->vc2013WinRtWin32CheckBox->isChecked();
}

bool WizardPageEnvOptions::isVc2013WinRtX64Selected() const
{
	return ui->vc2013WinRtX64CheckBox->isChecked();
}

bool WizardPageEnvOptions::isVc2013WinRtArmSelected() const
{
	return ui->vc2013WinRtArmCheckBox->isChecked();
}

/////////////////////////////////////////////////////////////

void WizardPageEnvOptions::updateNextButton( bool /*ignored*/ )
{
	bool vc2013PlatformChecked = isVc2013Win32Selected() || isVc2013X64Selected();
	bool vc2013RendererChecked = isVc2013DesktopGlSelected() || ( isVc2013Win32Selected() && isVc2013AngleSelected() );
	bool vc2013Good = ( ! mVc2013Enabled ) || ( vc2013PlatformChecked && vc2013RendererChecked );

	bool vc2013WinRtPlatformChecked = isVc2013WinRtWin32Selected() || isVc2013WinRtX64Selected() || isVc2013WinRtArmSelected();
	bool vc2013WinRtGood = ( ! mVc2013WinRtEnabled ) || vc2013WinRtPlatformChecked;

	wizard()->button( QWizard::NextButton )->setEnabled( vc2013Good && vc2013WinRtGood );
}

WizardPageEnvOptions::~WizardPageEnvOptions()
{
    delete ui;
}
