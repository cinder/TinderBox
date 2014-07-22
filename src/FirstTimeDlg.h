#ifndef FIRSTTIMEDLG_H
#define FIRSTTIMEDLG_H

#include <QDialog>

namespace Ui {
    class FirstTimeDlg;
}

class FirstTimeDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FirstTimeDlg(QWidget *parent = 0);
    ~FirstTimeDlg();

private:
    Ui::FirstTimeDlg *ui;
};

#endif // FIRSTTIMEDLG_H
