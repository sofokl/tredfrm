#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent, LkSettingsManager *man) :
    QDialog(parent), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    this->m_manager = man;

    ui->le_ServerAddress->setText(m_manager->getServerAddress());
    ui->le_User->setText(m_manager->getUserName());
    ui->le_Pass->setText(m_manager->getPassword());
    ui->sp_Port->setValue(m_manager->getPort());
    ui->cb_UseSsl->setChecked(m_manager->getUseSsl());
    ui->cb_IgnoreSslErrors->setChecked(m_manager->getIgnoreSslErrors());
    ui->cb_IgnoreSslErrors->setEnabled(ui->cb_UseSsl->isChecked());

}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_buttonBox_accepted()
{
    m_manager->setServerAddress(ui->le_ServerAddress->text());
                                //.replace(".00",".")
                                //.replace(".0", ".")
                               // .replace("..",".0."));
    m_manager->setUserName(ui->le_User->text());
    m_manager->setPassword(ui->le_Pass->text());
    m_manager->setPort(ui->sp_Port->value());
    m_manager->setUseSsl(ui->cb_UseSsl->isChecked());
    m_manager->setIgnoreSslErrors(ui->cb_IgnoreSslErrors->isChecked());
}

void SettingsDialog::on_cb_UseSsl_clicked()
{
    if(ui->cb_UseSsl->isChecked())
            ui->sp_Port->setValue(443);

    ui->cb_IgnoreSslErrors->setEnabled(ui->cb_UseSsl->isChecked());
}
