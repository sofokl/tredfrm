#include "providersdialog.h"
#include "ui_providersdialog.h"

ProvidersDialog::ProvidersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProvidersDialog)
{
    ui->setupUi(this);
}

ProvidersDialog::~ProvidersDialog()
{
    delete ui;
}
