#ifndef PROVIDERSDIALOG_H
#define PROVIDERSDIALOG_H

#include <QDialog>

namespace Ui {
class ProvidersDialog;
}

class ProvidersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProvidersDialog(QWidget *parent = 0);
    ~ProvidersDialog();

private:
    Ui::ProvidersDialog *ui;
};

#endif // PROVIDERSDIALOG_H
