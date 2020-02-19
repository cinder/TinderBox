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

#include "WizardPageEnvOptions.h"
#include "ui_WizardPageEnvOptions.h"
#include "WizardPageMain.h"

WizardPageEnvOptions::WizardPageEnvOptions( MainWizard *parent ) :
	QWizardPage( parent ), mParent( parent ),
    ui(new Ui::WizardPageEnvOptions)
{
    ui->setupUi(this);

	// VC 2015
	connect( ui->vc2015OpenGlCheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );
	connect( ui->vc2015AngleCheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );

	// VC 2015 WinRT
	connect( ui->vc2015WinRtWin32CheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );
	connect( ui->vc2015WinRtX64CheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );
	connect( ui->vc2015WinRtArmCheckBox,SIGNAL(clicked(bool)),this,SLOT(updateNextButton(bool)) );

#if ! defined( Q_OS_MAC )
	QFont font = ui->envOptionsLabel->font();
	font.setBold( true ); font.setPointSize( 12 );
	ui->envOptionsLabel->setFont( font );
	font.setPointSize( 10 );
	ui->vc2015HeaderLabel->setFont( font );
	ui->vc2015WinRtHeaderLabel->setFont( font );
	font.setPointSize( 9 );
	ui->vc2015RendererLabel->setFont( font );
	ui->vc2015WinRtPlatformLabel->setFont( font );
#endif
}

void WizardPageEnvOptions::initializePage()
{
	mVc2015Enabled = mParent->getWizardPageMain()->isVc2015Selected();
	mVc2015WinRtEnabled = mParent->getWizardPageMain()->isVc2015WinrtSelected();

	recursiveEnable( ui->vc2015Layout, mVc2015Enabled );
	recursiveEnable( ui->vc2015WinRtLayout, mVc2015WinRtEnabled );
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

bool WizardPageEnvOptions::isVc2015DesktopGlSelected() const
{
	return ui->vc2015OpenGlCheckBox->isChecked();
}

bool WizardPageEnvOptions::isVc2015AngleSelected() const
{
	return ui->vc2015AngleCheckBox->isChecked();
}

/////////////////////////////////////////////////////////////
// VC 2015 WinRT
bool WizardPageEnvOptions::isVc2015WinRtWin32Selected() const
{
	return ui->vc2015WinRtWin32CheckBox->isChecked();
}

bool WizardPageEnvOptions::isVc2015WinRtX64Selected() const
{
	return ui->vc2015WinRtX64CheckBox->isChecked();
}

bool WizardPageEnvOptions::isVc2015WinRtArmSelected() const
{
	return ui->vc2015WinRtArmCheckBox->isChecked();
}

/////////////////////////////////////////////////////////////

void WizardPageEnvOptions::updateNextButton( bool /*ignored*/ )
{
    bool vc2015RendererChecked = isVc2015DesktopGlSelected() || isVc2015AngleSelected();
	bool vc2015Good = ( ! mVc2015Enabled ) || ( vc2015RendererChecked );

	bool vc2015WinRtPlatformChecked = isVc2015WinRtWin32Selected() || isVc2015WinRtX64Selected() || isVc2015WinRtArmSelected();
	bool vc2015WinRtGood = ( ! mVc2015WinRtEnabled ) || vc2015WinRtPlatformChecked;

	wizard()->button( QWizard::NextButton )->setEnabled( vc2015Good && vc2015WinRtGood );
}

WizardPageEnvOptions::~WizardPageEnvOptions()
{
    delete ui;
}
