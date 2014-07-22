#include "FirstTimeDlg.h"
#include "ui_FirstTimeDlg.h"

FirstTimeDlg::FirstTimeDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FirstTimeDlg)
{
    ui->setupUi(this);
}

FirstTimeDlg::~FirstTimeDlg()
{
    delete ui;
}
