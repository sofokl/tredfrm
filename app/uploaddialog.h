#ifndef UPLOADDIALOG_H
#define UPLOADDIALOG_H

#include <QDialog>
#include <LkStoreHelper.h>

typedef QMap<QString, QVariant> TData;

namespace Ui {
class UploadDialog;
}

class UploadDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit UploadDialog(LkStoreHelper* helper, QWidget *parent = 0);
    ~UploadDialog();

private slots:
    void on_tbSelectFile_clicked();
    void on_pushButton_clicked();
    void setProvidersList();
    void setSalepointList();

private:
    Ui::UploadDialog *ui;
    LkStoreHelper *m_helper;
};

#endif // UPLOADDIALOG_H
