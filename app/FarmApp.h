#ifndef FARMAPP_H
#define FARMAPP_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSortFilterProxyModel>
#include <QProgressDialog>
#include <QUuid>

#include <LkSpinBoxDelegate.h>
#include <LkSqlTableModel.h>
#include <LkSettingsManager.h>
#include <LkStoreHelper.h>
#include <LkUpdateHelper.h>
#include <transportproxy.h>
#include "diskid32.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool needExit;

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void getPriceListUpdate();


public slots:
    void selectAllModels(int selected_order=0);
    void dataUpdateSuccess(const QString &part, int step=-1);
    void orderChanged(int, bool show_progrees=false);
    void operationError(QString title, QString text, bool canceled);
    void updateAllTables();
    
private slots:

    void priceDataIsGetted(int count);
    bool init_db(QString);
    void init_models();
    void init_salePoints_comboBox();
    void init_medicines_view();
    void init_price_view();
    void init_orders_view();
    void init_order_details_view();

    void currentProductChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void currentOrderChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void currentSalepointChanged(int index);

    void on_tableView_Price_doubleClicked(QModelIndex index);
    void on_tableView_CurrentOrder_doubleClicked(QModelIndex index);
    void on_pb_DeleteOrder_clicked();
    void on_pb_ClearOrder_clicked();
    void on_pb_SendAllOrder_clicked();
    void on_pb_ShowSended_toggled(bool checked);
    void on_pb_Search_clicked();
    void on_le_FilterMedicines_textChanged(const QString &arg1);
    void on_pb_UpdatePriceLists_clicked();
    void on_actionSettings_triggered();
    void on_actionUploadOrders_triggered();
    void on_actionCheckUpdate_triggered();
    void on_action_Qt_triggered();
    void on_actionAbout_triggered();

    void priceCommitDataOrders(QWidget *editor);
    void createTransport();
    void createDialog(QString startLabel, int range);

private:
    Ui::MainWindow *ui;

    QSqlDatabase db;
    LkSqlTableModel *m_medicines_model, *m_price_model, *m_orders_model, *m_details_model;
    QSortFilterProxyModel* m_proxy_medicine, * m_proxy_price, *m_proxy_details;
    QSqlTableModel *m_salePoints_model;
    LkSpinBoxDelegate *price_delegate;

    TransportProxy *m_1c_proxy;
    LkSettingsManager * m_manager;
    LkStoreHelper *m_helper;
    LkUpdateHelper *m_updater;
    int m_lastRowProduct;
    int m_priceDataCount;
    bool m_createDB;
    bool m_isActivated;
    QUuid m_snUID;

    QProgressDialog *m_progress_dlg;

};

#endif // FARMAPP_H
