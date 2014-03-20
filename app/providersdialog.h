#ifndef PROVIDERSDIALOG_H
#define PROVIDERSDIALOG_H

#include <QDialog>
#include <LkSettingsManager.h>
#include <LkStoreHelper.h>
#include <LkSpinBoxDelegate.h>
#include <LkCheckedDelegate.h>
#include <LkSqlTableModel.h>
#include <QSortFilterProxyModel>

#include <transportproxy.h>

namespace Ui {
class ProvidersDialog;
}

class ProvidersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProvidersDialog(QWidget *parent = 0,LkSettingsManager *settings=0, QSqlDatabase *db=0);
    ~ProvidersDialog();
    void initModel();

signals:
    void providersListNeedUpdate();

private slots:
    void on_pushButton_Update_clicked();
    void on_checkBox_toggled(bool checked);
    void updateModel();

private:
    Ui::ProvidersDialog *ui;
    LkSettingsManager *m_settings;
    QSortFilterProxyModel *m_filter_model;
    LkSqlTableModel *m_model;
    LkStoreHelper *m_helper;
    LkSpinBoxDelegate *coef_delegate;
    LkCheckedDelegate *used_delegate;
};


#endif // PROVIDERSDIALOG_H
