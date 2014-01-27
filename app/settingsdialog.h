#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <LkSettingsManager.h>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SettingsDialog(QWidget *parent = 0, LkSettingsManager *man = 0);
    ~SettingsDialog();
    
private slots:
    void on_buttonBox_accepted();

    void on_cb_UseSsl_clicked();

private:
    Ui::SettingsDialog *ui;
    LkSettingsManager *m_manager;


};

#endif // SETTINGSDIALOG_H
