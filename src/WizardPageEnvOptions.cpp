#include "WizardPageEnvOptions.h"
#include "ui_WizardPageEnvOptions.h"

WizardPageEnvOptions::WizardPageEnvOptions(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::WizardPageEnvOptions)
{
    ui->setupUi(this);
}

WizardPageEnvOptions::~WizardPageEnvOptions()
{
    delete ui;
}
