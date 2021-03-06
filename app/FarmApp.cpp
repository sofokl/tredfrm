#include "FarmApp.h"
#include "ui_MainWindow.h"

#include <QItemSelection>
#include <QFile>
#include <QWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QByteArray>
#include <QSqlError>
#include <QProgressDialog>
#include <QStandardPaths>
#include <QDir>
#include <QTimer>
#include <QProcess>
#include <QDataWidgetMapper>
#include <QToolButton>

#include "uploaddialog.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {

    ui->setupUi(this);

    needExit = false;
    m_lastRowProduct = 0;
    m_1c_proxy = 0;

    diskid::SYSTEMIDSDATA geom = sprintf_sn();

    QString gg = QString("%1-%2").arg(geom.system_id).arg(geom.HardDriveSerialNumber);

    QUuid ns = QUuid(gg);
    QByteArray name = QByteArray(gg.toLatin1());

    m_snUID = QUuid::createUuidV3(ns, name);

    m_manager = new LkSettingsManager(this);

    QString clientName= m_manager->getClientName();
    if(!clientName.isEmpty())
        this->setWindowTitle(tr("%1 - ТредиФарм").arg(clientName));

    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);

    if(QDir(path).exists())
        path.append("/data.bin");
    else{
        QDir dir = QDir::root();
        dir.mkdir(path);
    }

    if(!QFile(path).exists() || QFile(path).size() == 0){
        m_createDB = true;
    } else {
        m_createDB = false;
    }

    m_isActivated = init_db(path);

    if(needExit)
            return;

    m_updater = new LkUpdateHelper(m_manager, BUILDN, &db, this);

    connect(m_helper, SIGNAL(orderIsChanged(int, bool)), this, SLOT(orderChanged(int,bool)));

    ui->splitter_PriceGroup->restoreState(m_manager->getSplitterState("0x01"));
    ui->splitter_2_Bottom->restoreState(m_manager->getSplitterState("0x02"));
    ui->splitter_Main->restoreState(m_manager->getSplitterState("0x03"));
    ui->tableView_Medicines->horizontalHeader()->restoreState(m_manager->getSplitterState("0x11"));
    //ui->tableView_Price->horizontalHeader()->restoreState(m_manager->getSplitterState("0x12"));
    ui->tableView_Orders->horizontalHeader()->restoreState(m_manager->getSplitterState("0x13"));
    ui->tableView_CurrentOrder->horizontalHeader()->restoreState(m_manager->getSplitterState("0x14"));

    init_models();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createTransport(){

    m_1c_proxy = new TransportProxy(m_snUID.toString().replace("{","").replace("}",""), this);
    m_1c_proxy->setUserName(m_manager->getUserName());
    m_1c_proxy->setPassword(m_manager->getPassword());
    m_1c_proxy->useSsl = m_manager->getUseSsl();
    m_1c_proxy->ignoreSslErrors = m_manager->getIgnoreSslErrors();
    m_1c_proxy->setEndPoint(m_manager->getServerAddress(), m_manager->getPort());

    connect(this, SIGNAL(getPriceListUpdate(QList<QString>)), m_1c_proxy, SLOT(getPriceList(QList<QString>))); //,Qt::QueuedConnection
    connect(m_1c_proxy, SIGNAL(priceDataIsReady(TList,TList,TList,TList)), m_helper, SLOT(writeDataPrice(TList,TList,TList,TList)));
    connect(m_1c_proxy, SIGNAL(priceDataIsGetted(int)), this, SLOT(priceDataIsGetted(int)));
    connect(m_1c_proxy, SIGNAL(dataSalepoinstIsReady(TList)),m_helper, SLOT(writeSalePoints(TList)));
    connect(m_1c_proxy, SIGNAL(orderIsSended(int,QString)), m_helper, SLOT(writeOrderAsSended(int, QString)));
    connect(m_1c_proxy, SIGNAL(dataProvidersIsReady(TList)), m_helper, SLOT(writeProviders(TList)));
    connect(m_1c_proxy, SIGNAL(transportError(QString,QString,bool)), this, SLOT(operationError(QString,QString,bool)));

}

void MainWindow::createDialog(QString startLabel, int range) {

    m_progress_dlg = new QProgressDialog(this);
    m_progress_dlg->setModal(true);
    m_progress_dlg->setCancelButton(0);
    m_progress_dlg->setRange(0, range);
    m_progress_dlg->setWindowTitle(tr("Пожалуйста подождите..."));
    m_progress_dlg->setMinimumDuration(100);
    m_progress_dlg->setFixedWidth(250);
    m_progress_dlg->setAutoClose(true);
    m_progress_dlg->setAutoReset(true);
    m_progress_dlg->setUpdatesEnabled(true);
    m_progress_dlg->setLabelText(startLabel);

}

bool MainWindow::init_db(QString path)
{

#ifdef USE_CRYPT_DB
    db = QSqlDatabase::addDatabase("QSQLCIPHER", "local_conn2");
#else
    db = QSqlDatabase::addDatabase("QSQLITE", "local_conn2");
#endif
    if(db.lastError().type() != QSqlError::NoError){
        needExit = true;
        QMessageBox::critical(this, tr("DataBase Error"), db.lastError().text());
        return false;
    }

    db.setDatabaseName(path);

    if(!db.open()) {
        needExit = true;
        QMessageBox::critical(this, tr("Ошибка открытия базы данных"), db.lastError().text(), QMessageBox::Ok);
        db.close();
        return false;
    }

    m_helper = new LkStoreHelper(this, db);

    QSqlError er = m_helper->startDataBase();
    if(er.type() != QSqlError::NoError) {
        needExit = true;
        QMessageBox::critical(this, tr("Start database"), er.text());
        return false;
    }

    connect(m_helper, SIGNAL(dataIsWriten(QString,int)), this, SLOT(dataUpdateSuccess(QString, int)));

    if(m_createDB == true) {
        QFile file(":/etc/script");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() <<  "Cannot open resourse a file";
        }

        m_helper->writeNewDbSchema(file.readAll());
    }

    return m_helper->chechState(m_snUID.toString());

}

void MainWindow::init_models()
{
    init_medicines_view();
    init_price_view();
    init_orders_view();
    init_order_details_view();

    connect(ui->tableView_Medicines->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(currentProductChanged(QItemSelection,QItemSelection)));
    connect(ui->tableView_Orders->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(currentOrderChanged(QItemSelection,QItemSelection)));
    connect(ui->tableView_Price, SIGNAL(editFinished(int)), this, SLOT(priceCommitDataOrders(int)));

    connect(ui->cb_SalePoints,SIGNAL(currentIndexChanged(int)), this, SLOT(currentSalepointChanged(int)));

    init_salePoints_comboBox();
}

void MainWindow::init_salePoints_comboBox()
{

    m_salePoints_model = new QSqlTableModel(this, db);
    m_salePoints_model->setTable("salepoints");
    m_salePoints_model->setEditStrategy(QSqlTableModel::OnFieldChange);
    m_salePoints_model->setFilter("is_deleted=0");
    m_salePoints_model->setSort(m_salePoints_model->fieldIndex("is_default"), Qt::DescendingOrder);

    ui->cb_SalePoints->setModel(m_salePoints_model);
    ui->cb_SalePoints->setModelColumn(m_salePoints_model->fieldIndex("name"));

    mapper = new QDataWidgetMapper(this);
    mapper->setModel(m_salePoints_model);
    mapper->addMapping(ui->dsb_markups, m_salePoints_model->fieldIndex("markups"));
    mapper->addMapping(ui->cb_SalePoints, m_salePoints_model->fieldIndex("name"));

    m_salePoints_model->select();
    ui->cb_SalePoints->setCurrentIndex(0);

}

void MainWindow::init_medicines_view()
{
    m_medicines_model = new LkSqlTableModel(this, db);
    m_medicines_model->setTable("V01");
    m_medicines_model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    QSqlError er = m_medicines_model->lastError();

    if(er.type() != QSqlError::NoError) {
        QMessageBox::critical(this, tr("Ошибка открытия таблицы"), tr("Ошибка %1\n%2").arg( QString::number(er.type()), er.text()), QMessageBox::Close);
    }

    m_medicines_model->setHeaderData(0, Qt::Horizontal, tr("Код"));
    m_medicines_model->setHeaderData(1, Qt::Horizontal, tr("Пункт"));
    m_medicines_model->setHeaderData(2, Qt::Horizontal, tr("Наименование"));
    m_medicines_model->setHeaderData(3, Qt::Horizontal, tr("ЖВП"));


    m_medicines_model->setRowsIcon(QIcon(":/icons/item"), 2);
    m_medicines_model->setColorTextByValueColumn(3, QColor(Qt::darkGreen));

    m_proxy_medicine = new QSortFilterProxyModel(this);
    m_proxy_medicine->setSourceModel(m_medicines_model);
    m_proxy_medicine->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxy_medicine->setFilterKeyColumn(2);
    m_proxy_medicine->setDynamicSortFilter(true);

    ui->tableView_Medicines->setModel(m_proxy_medicine);
    ui->tableView_Medicines->setSortingEnabled(true);
    ui->tableView_Medicines->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_Medicines->setEditTriggers(QTableView::NoEditTriggers);
    ui->tableView_Medicines->setColumnHidden(0, true);
    ui->tableView_Medicines->setColumnHidden(1, true);
    ui->tableView_Medicines->setColumnHidden(3, true);
    ui->tableView_Medicines->setColumnHidden(4, true);
    ui->tableView_Medicines->setItemDelegateForColumn(3, new LkCheckedDelegate(ui->tableView_Medicines));
    ui->tableView_Medicines->setItemDelegateForColumn(4, new LkCheckedDelegate(ui->tableView_Medicines));
    ui->tableView_Medicines->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_Medicines->verticalHeader()->setVisible(false);
    ui->tableView_Medicines->verticalHeader()->setDefaultSectionSize(18);
    ui->tableView_Medicines->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableView_Medicines->setIconSize(QSize(13,13));
    ui->tableView_Medicines->sortByColumn(2, Qt::AscendingOrder);
   // ui->tableView_Medicines->setAlternatingRowColors(true);

}

void MainWindow::init_price_view()
{
    m_query_model = new LkSqlQueryModel(this);
    QString textQuery = m_helper->getTextPriceView();

    QSqlQuery query(db);
    query.prepare(textQuery);
    m_query_model->setQuery(query);
    m_query_model->exec(2,"");

    QSqlError er = m_query_model->lastError();

    if(er.type() != QSqlError::NoError)
        QMessageBox::critical(this, tr("Ошибка открытия таблицы"), tr("Ошибка %1\n%2").arg( QString::number(er.type()), er.text()), QMessageBox::Close);

    int hideFlag = LkInputTableView::hideColumnFlag;
    m_query_model->setHeaderData(0, Qt::Horizontal, tr("КодТовара"));
    m_query_model->setHeaderData(0, Qt::Horizontal, hideFlag, Qt::UserRole);

    m_query_model->setHeaderData(1, Qt::Horizontal, tr("ИдСинонима"));
    m_query_model->setHeaderData(1, Qt::Horizontal, hideFlag, Qt::UserRole);

    m_query_model->setHeaderData(2, Qt::Horizontal, tr("Поставщик"));

    m_query_model->setHeaderData(3, Qt::Horizontal, tr("Дата прайса"));
    m_query_model->setHeaderData(3, Qt::Horizontal, hideFlag, Qt::UserRole);

    m_query_model->setHeaderData(4, Qt::Horizontal, tr("Наименование"));
    m_query_model->setHeaderData(4, Qt::Horizontal, hideFlag, Qt::UserRole);

    m_query_model->setHeaderData(5, Qt::Horizontal, tr("Код"));
    m_query_model->setHeaderData(5, Qt::Horizontal, hideFlag, Qt::UserRole);

    m_query_model->setHeaderData(6, Qt::Horizontal, tr("Наименование товара"));
    m_query_model->setHeaderData(7, Qt::Horizontal, tr("Производитель"));
    m_query_model->setHeaderData(8, Qt::Horizontal, tr("Рег. Цена"));
    m_query_model->setHeaderData(8, Qt::Horizontal, tr("Регистр-ая цена"), Qt::ToolTipRole);

    m_query_model->setHeaderData(9, Qt::Horizontal, tr("Группа"));
    m_query_model->setHeaderData(9, Qt::Horizontal, hideFlag, Qt::UserRole);
    m_query_model->setHeaderData(10, Qt::Horizontal, tr("КЦУ"));
    m_query_model->setHeaderData(10, Qt::Horizontal, tr("Коеффициент ценовых упаковок"), Qt::ToolTipRole);
    m_query_model->setHeaderData(11, Qt::Horizontal, tr("Годен до"));
    m_query_model->setHeaderData(12, Qt::Horizontal, tr("Цена по коэф."));
    m_query_model->setHeaderData(13, Qt::Horizontal, tr("Цена с НДС"));
    m_query_model->setHeaderData(14, Qt::Horizontal, tr("Остаток"));
    m_query_model->setHeaderData(15, Qt::Horizontal, tr("Заказано"));
    m_query_model->setHeaderData(16, Qt::Horizontal, tr("Продажная цена"));
    m_query_model->setHeaderData(17, Qt::Horizontal, tr("Точка продажи"));
    m_query_model->setHeaderData(17, Qt::Horizontal, hideFlag, Qt::UserRole);
    m_query_model->setHeaderData(18, Qt::Horizontal, tr("Сроковый товар"));
    m_query_model->setHeaderData(18, Qt::Horizontal, hideFlag, Qt::UserRole);
    m_query_model->setHeaderData(19, Qt::Horizontal, tr("Уцененный товар"));
    m_query_model->setHeaderData(19, Qt::Horizontal, hideFlag, Qt::UserRole);

    m_query_model->setColorTextByValueColumn(19, QColor(Qt::darkRed));

    m_query_model->setVerticalHeadersIcon(QIcon(":/icons/bul_blue"), QIcon(":/icons/bul_orange"));
    m_query_model->setAlternateIconColumnSource(18);
    m_query_model->setAlternatebackgroundBySource(11, 18, QBrush(QColor(255, 204, 0)));

    m_proxy_price = new QSortFilterProxyModel(this);
    m_proxy_price->setSourceModel(m_query_model);
    m_proxy_price->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxy_price->setDynamicSortFilter(true);

    if(!m_manager->getSplitterState("0x12").isEmpty())
        ui->tableView_Price->horizontalHeader()->restoreState(m_manager->getSplitterState("0x12"));

    ui->tableView_Price->setModel(m_proxy_price);

    ui->tableView_Price->setSortingEnabled(true);
    ui->tableView_Price->verticalHeader()->setDefaultSectionSize(18);
    ui->tableView_Price->horizontalHeader()->setDefaultSectionSize(20);
    ui->tableView_Price->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_Price->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    //ui->tableView_Price->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Fixed);
    ui->tableView_Price->horizontalHeader()->setSectionsMovable(true);
    ui->tableView_Price->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_Price->sortByColumn(12, Qt::AscendingOrder);
    ui->tableView_Price->setIconSize(QSize(8,8));

    if(m_manager->getSplitterState("0x12").isEmpty()){
        ui->tableView_Price->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
        ui->tableView_Price->setColumnWidth(8, 50);
        ui->tableView_Price->setColumnWidth(10, 40);
    }
    else
        ui->tableView_Price->horizontalHeader()->restoreState(m_manager->getSplitterState("0x12"));
}

void MainWindow::init_orders_view()
{
    m_orders_model = new LkSqlTableModel(this, db);
    m_orders_model->setTable("V03");
    m_orders_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_orders_model->setFilter("sended=0");
    m_orders_model->select();

    QSqlError er = m_orders_model->lastError();

    if(er.type() != QSqlError::NoError)
        QMessageBox::critical(this, tr("Ошибка открытия таблицы"), tr("Ошибка %1\n%2").arg( QString::number(er.type()), er.text()), QMessageBox::Close);

    m_orders_model->setHeaderData(0, Qt::Horizontal, tr("id"));
    m_orders_model->setHeaderData(1, Qt::Horizontal, tr("КодПоставщика"));
    m_orders_model->setHeaderData(2, Qt::Horizontal, tr("КодПунктаДоставки"));
    m_orders_model->setHeaderData(3, Qt::Horizontal, tr("Отправлен"));
    m_orders_model->setHeaderData(4, Qt::Horizontal, tr("Номер"));
    m_orders_model->setHeaderData(5, Qt::Horizontal, tr("Дата"));
    m_orders_model->setHeaderData(6, Qt::Horizontal, tr("Поставщик"));
    m_orders_model->setHeaderData(7, Qt::Horizontal, tr("Пункт доставки"));
    m_orders_model->setHeaderData(8, Qt::Horizontal, tr("Позиций"));
    m_orders_model->setHeaderData(9, Qt::Horizontal, tr("Сумма"));

    m_orders_model->setRowsIcon(QIcon(":/icons/bul_red"), 5);
    m_orders_model->setColumnAlignment(6, Qt::AlignCenter);
    m_orders_model->setColumnAlignment(7, Qt::AlignCenter);

    m_orders_model->setColumnAlignment(8, Qt::AlignRight);
    m_orders_model->setColumnAlignment(9, Qt::AlignRight);

    ui->tableView_Orders->setModel(m_orders_model);
    ui->tableView_Orders->setSortingEnabled(false);
    ui->tableView_Orders->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_Orders->setEditTriggers(QTableView::NoEditTriggers);
    ui->tableView_Orders->hideColumn(0);
    ui->tableView_Orders->hideColumn(1);
    ui->tableView_Orders->hideColumn(2);
    ui->tableView_Orders->hideColumn(3);
    ui->tableView_Orders->setColumnHidden(4, true);
    ui->tableView_Orders->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_Orders->verticalHeader()->setVisible(false);
    ui->tableView_Orders->verticalHeader()->setDefaultSectionSize(18);
    ui->tableView_Orders->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableView_Orders->setIconSize(QSize(13,13));

}

void MainWindow::init_order_details_view()
{
    m_details_model = new LkSqlTableModel(this, db);
    m_details_model->setTable("V04");
    m_details_model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    QSqlError er = m_details_model->lastError();

    if(er.type() != QSqlError::NoError)
        QMessageBox::critical(this, tr("Ошибка открытия таблицы"),
                              tr("Ошибка %1\n%2").arg( QString::number(er.type()), er.text()),
                              QMessageBox::Close);

    int hideFlag = LkInputTableView::hideColumnFlag;

    m_details_model->setHeaderData(0, Qt::Horizontal, tr("id"));
    m_details_model->setHeaderData(0, Qt::Horizontal, hideFlag, Qt::UserRole);
    m_details_model->setHeaderData(1, Qt::Horizontal, tr("order_id"));
    m_details_model->setHeaderData(1, Qt::Horizontal, hideFlag, Qt::UserRole);
    m_details_model->setHeaderData(2, Qt::Horizontal, tr("syn_id"));
    //m_details_model->setHeaderData(2, Qt::Horizontal, hideFlag, Qt::UserRole);
    m_details_model->setHeaderData(3, Qt::Horizontal, tr("Наименование"));
    m_details_model->setHeaderData(3, Qt::Horizontal, hideFlag, Qt::UserRole);
    m_details_model->setHeaderData(4, Qt::Horizontal, tr("Наименование товара"));//Синоним
    m_details_model->setHeaderData(5, Qt::Horizontal, tr("Срок годности"));
    m_details_model->setHeaderData(6, Qt::Horizontal, tr("Поставщик"));
    m_details_model->setHeaderData(6, Qt::Horizontal,  hideFlag, Qt::UserRole);
    m_details_model->setHeaderData(7, Qt::Horizontal, tr("Кол-во"));
    m_details_model->setHeaderData(8, Qt::Horizontal, tr("Цена закупочная"));
    m_details_model->setHeaderData(9, Qt::Horizontal, tr("Сумма"));
    m_details_model->setHeaderData(10, Qt::Horizontal, tr("ПунктДоставки"));
    m_details_model->setHeaderData(10, Qt::Horizontal,  hideFlag, Qt::UserRole);
    m_details_model->setHeaderData(11, Qt::Horizontal, tr("Заказ отправлен"));
    m_details_model->setHeaderData(11, Qt::Horizontal,  hideFlag, Qt::UserRole);
    m_details_model->setColumnAlignment(4, Qt::AlignLeft);
    m_details_model->setColumnAlignment(7, Qt::AlignRight);
    m_details_model->setColumnAlignment(8, Qt::AlignRight);
    m_details_model->setColumnAlignment(9, Qt::AlignRight);

   // m_details_model->setVerticalHeadersIcon(QIcon(":/icons/bul_blue"), QIcon(":/icons/bul_orange"));

    m_proxy_details = new QSortFilterProxyModel(this);
    m_proxy_details->setSourceModel(m_details_model);
    m_proxy_details->setFilterKeyColumn(1);                 //Фильтовать будем по колонке (order_id)ID наряда

    m_proxy_details->setDynamicSortFilter(true);
    m_proxy_details->setFilterFixedString("__"); // Сразу применим фильтр
    m_proxy_details->sort(4);

    ui->tableView_CurrentOrder->setModel(m_proxy_details);
    ui->tableView_CurrentOrder->setSortingEnabled(false);
    ui->tableView_CurrentOrder->setEditTriggers(QTableView::NoEditTriggers);
    ui->tableView_CurrentOrder->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_CurrentOrder->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_CurrentOrder->horizontalHeader()->setStretchLastSection(true);
    //ui->tableView_CurrentOrder->verticalHeader()->setVisible(false);
    ui->tableView_CurrentOrder->verticalHeader()->setDefaultSectionSize(18);
    ui->tableView_CurrentOrder->horizontalHeader()->resizeSections(QHeaderView::Interactive);
}

void MainWindow::selectAllModels(int selected_order) {

    if(m_medicines_model->rowCount() <= 0)
        m_lastRowProduct = -1;

    ui->tableView_Medicines->selectRow(m_lastRowProduct);

    if(m_query_model->rowCount() <= 0) {
        ui->gb_CurrentOrder->setEnabled(false);
    }
    else {
        ui->tableView_Orders->selectRow(selected_order);
    }

    m_details_model->select();
    ui->tableView_Medicines->setFocus();

}

void MainWindow::currentOrderChanged(const QItemSelection & selected, const QItemSelection &) {

    if(selected.isEmpty()) {
        m_proxy_details->setFilterFixedString("__");
        ui->gb_CurrentOrder->setEnabled(false);
        return;
    }

    QString order_id = selected.indexes().at(0).data().toString();
    m_proxy_details->setFilterFixedString(order_id);
    m_details_model->select();
    ui->gb_CurrentOrder->setEnabled(ui->tableView_CurrentOrder->verticalHeader()->count() > 0);

}

void MainWindow::currentProductChanged(const QItemSelection & selected, const QItemSelection &) {

    if(selected.isEmpty()) {
        m_query_model->exec(2, "");
        return;
    }

    QString med_key = selected.indexes().at(0).data().toString();

    m_query_model->exec(2, med_key);

    m_lastRowProduct = selected.indexes().at(0).row();

    while (m_query_model->canFetchMore()) {
         m_query_model->fetchMore();
    }

}

void MainWindow::currentSalepointChanged(int index) {

    QModelIndex cur_index = m_salePoints_model->index(index, 0);
    QString code = cur_index.data().toString();

    m_helper->setCurrentSpCode(code);
    mapper->setCurrentModelIndex(cur_index);

    QString filter(QString("salepoint_code='%1'").arg(code));

    QString filter_order = filter + QString(" And sended=%1").arg(ui->pb_ShowSended->isChecked());
    m_orders_model->setFilter(filter_order);

    updateAllTables();

}

void MainWindow::on_tableView_CurrentOrder_doubleClicked(QModelIndex index) {

    QModelIndex synonim_index = ui->tableView_CurrentOrder->model()->index(index.row(), 2);
    QString synonim_id = synonim_index.data().toString();

    //Get synonim_id in price model
    QModelIndex start_ind = m_query_model->index(0,1);
    QModelIndexList lst = m_query_model->match(start_ind, Qt::DisplayRole,
                                               synonim_id, -1, Qt::MatchFixedString);
    if(lst.count() == 0)
        return;

    //Get product_id
    QString product_id = m_query_model->index(lst.at(0).row(), 0).data().toString();

    //Get index product for product_id
    int prod_id_column = 0;
    start_ind = m_proxy_medicine->index(0, prod_id_column);
    lst = m_proxy_medicine->match(start_ind, Qt::DisplayRole, product_id, 1, Qt::MatchFixedString);
    if(lst.count() == 0)
        return;

    //select produc
    ui->tableView_Medicines->setCurrentIndex(lst.at(0));

    //get synonim row from filtered model
    start_ind = m_proxy_price->index(0,1);
    lst = m_proxy_price->match(start_ind, Qt::DisplayRole, synonim_id, 1, Qt::MatchFixedString);
    if(lst.count() == 0)
        return;

    ui->tableView_Price->setCurrentIndex(lst.at(0));
    ui->tableView_Price->setFocus();

}

void MainWindow::on_tableView_Price_doubleClicked(QModelIndex index) {

    ui->tableView_Price->keyboardSearch(m_proxy_price->index(index.row(), ORDERCOUNT_COLUMN).data().toString());

}

void MainWindow::on_le_FilterMedicines_textChanged(const QString &arg1) {

    if(!arg1.isEmpty())
        ui->pb_Search->setIcon(QIcon(":/icons/clear"));
    else
        ui->pb_Search->setIcon(QIcon(":/icons/search"));

    QRegExp regExp(arg1, Qt::CaseInsensitive, QRegExp::FixedString);

    if(!ui->tableView_Medicines->model())
        return;

    m_proxy_medicine->setFilterRegExp(regExp);
    ui->tableView_Medicines->selectRow(0);

}

void MainWindow::on_pb_ShowSended_toggled(bool checked) {

    if(!m_orders_model || !m_salePoints_model)
        return;

    QModelIndex cur_index = m_salePoints_model->index(ui->cb_SalePoints->currentIndex(), 0);
    QString sp_code = cur_index.data().toString();

    QString filter(QString("salepoint_code='%1'").arg(sp_code));
    filter += QString(" And sended=%1").arg(checked);

    m_orders_model->setFilter(filter);

    if(checked) {
        m_orders_model->setRowsIcon(QIcon(":/icons/bul_green"), 4);
        ui->pb_ShowSended->setIcon(QIcon(":/icons/script_lightning"));
        ui->pb_ShowSended->setText(tr("Отправленные заказы"));
        ui->tableView_Orders->setColumnHidden(4, false);
    }
    else {
        m_orders_model->setRowsIcon(QIcon(":/icons/bul_red"), 5);
        ui->pb_ShowSended->setIcon(QIcon(":/icons/script"));
        ui->pb_ShowSended->setText(tr("Заказы в работе"));
        ui->tableView_Orders->setColumnHidden(4, true);
    }

    selectAllModels();

    //ui->tableView_Orders->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableView_Orders->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_Orders->setFocus();

}

void MainWindow::on_pb_Search_clicked() {
    ui->le_FilterMedicines->clear();
    ui->pb_Search->setIcon(QIcon(":/icons/search"));
}

void MainWindow::on_pb_DeleteOrder_clicked() {

    if(!ui->tableView_Orders->model())
        return;

    QModelIndex c_index = ui->tableView_Orders->selectionModel()->currentIndex();
    if(!c_index.isValid()){
        QMessageBox::information(this, tr("Удаление заказа не выполнено"),
                                 tr("Не выбран заказ для удаления!"), QMessageBox::Ok);
        return;
    }

    QModelIndex index_id = m_orders_model->index(c_index.row(),0);
    QString order_id = index_id.data().toString();

    QSqlError error = m_helper->deleteOrder(order_id);
    if(error.type() != QSqlError::NoError)
        QMessageBox::critical(this, tr("Не удалось удалить заказ\t\t\t\t"), error.text(), QMessageBox::Close);

    currentProductChanged(ui->tableView_Medicines->selectionModel()->selection(),
                          ui->tableView_Medicines->selectionModel()->selection());

    m_orders_model->select();

    if(m_orders_model->rowCount() == 0)
    {
        m_proxy_details->setFilterFixedString("__");
        ui->gb_CurrentOrder->setEnabled(false);
    }
    else
        ui->tableView_Orders->selectRow(0);

    ui->tableView_Orders->setFocus();
}

void MainWindow::on_pb_ClearOrder_clicked() {

    if(!m_orders_model)
        return;

    QModelIndex c_index = ui->tableView_Orders->selectionModel()->currentIndex();

    if(!c_index.isValid()){
        QMessageBox::information(this, tr("Очистка заказа не выполнена"),
                                 tr("Не выбран заказ или он пуст!"), QMessageBox::Ok);
        return;
    }

    QModelIndex index_id = m_orders_model->index(c_index.row(),0);
    QString order_id = index_id.data().toString();
    QSqlError error = m_helper->clearOrder(order_id);
    if(error.type() != QSqlError::NoError)
        QMessageBox::critical(this, tr("Не удалось очистить заказ\t\t\t\t"), error.text(), QMessageBox::Close);

    currentProductChanged(ui->tableView_Medicines->selectionModel()->selection(),
                          ui->tableView_Medicines->selectionModel()->selection());


    m_orders_model->select();

    if(m_orders_model->rowCount() == 0)
        m_details_model->select();
    else
        ui->tableView_Orders->selectRow(c_index.row());

    ui->tableView_Orders->setFocus();
}

void MainWindow::on_pb_UpdatePriceLists_clicked() {

    createTransport();
    createDialog(tr("Шаг 1/4\nПодключение к серверу данных ..."), 30);
    m_progress_dlg->show();

    emit getPriceListUpdate(m_helper->getUsedProviderList());
}

void MainWindow::priceDataIsGetted(int count) {

    m_priceDataCount = count;
    m_progress_dlg->setLabelText(tr("Шаг 3/4\nРаспаковка данных ..."));
    m_progress_dlg->setValue(15);
    QCoreApplication::processEvents();

}

void MainWindow::dataUpdateSuccess(const QString &part, int step) {

    if(part == QString("SalePoints")){
        m_progress_dlg->setLabelText(tr("Шаг 2/4\nПолучение данных ..."));
        m_progress_dlg->setValue(1);
        QCoreApplication::processEvents();
        return;
    }

    if(part == QString("Finish")){
        m_lastRowProduct = 0;

        m_salePoints_model->select();
        ui->cb_SalePoints->setCurrentIndex(0);
        ui->tableView_Medicines->setFocus();
        ui->tableView_Price->sortByColumn(12);
        m_progress_dlg->accept();
        QMessageBox::information(this, tr("Обновление выполнено"), tr("Прайс лист обновлен"), QMessageBox::Ok);

        delete m_1c_proxy;
        return;
    }

//    if(part == QString("Providers")){
//        m_progress_dlg->accept();
//        QMessageBox::information(this, tr("Обновление выполнено"), tr("Список поставщиков обновлен"), QMessageBox::Ok);
//        emit providersIsUpdated();
//        //delete m_1c_proxy;
//        return;
//    }

    if(part == QString("PriceList"))
    {
        m_progress_dlg->setLabelText(tr("Шаг 4/4\nЗапись данных ..."));
        if(step == 0) {
            m_progress_dlg->setRange(1, m_priceDataCount);
        }
        m_progress_dlg->setValue(step);
        QCoreApplication::processEvents();
    }

}

void MainWindow::on_actionSettings_triggered() {

    SettingsDialog *dialog = new SettingsDialog(this, m_manager);
    dialog->setWindowModality(Qt::WindowModal);
    dialog->exec();

}

void MainWindow::on_pb_SendAllOrder_clicked() {

    QCoreApplication::processEvents();
    TDataList data = m_helper->getOrdersData(QVector<int>());

    if(data.count() == 0) {
        QMessageBox::information(this, tr("Отправка данных"), tr("Нечего отправлять!"), QMessageBox::Ok);
        return;
    }

    createDialog(tr("Отправка данных ..."), data.count()+1);

    m_progress_dlg->show();
    m_progress_dlg->setValue(1);
    QCoreApplication::processEvents();

    createTransport();
    m_1c_proxy->sendOrders(data);

    delete m_1c_proxy;

}

void MainWindow::closeEvent(QCloseEvent *event) {

    db.close();

    m_manager->setSplitterState("0x01", ui->splitter_PriceGroup->saveState());
    m_manager->setSplitterState("0x02", ui->splitter_2_Bottom->saveState());
    m_manager->setSplitterState("0x03", ui->splitter_Main->saveState());

    m_manager->setSplitterState("0x11", ui->tableView_Medicines->horizontalHeader()->saveState());
    m_manager->setSplitterState("0x12", ui->tableView_Price->horizontalHeader()->saveState());
    m_manager->setSplitterState("0x13", ui->tableView_Orders->horizontalHeader()->saveState());
    m_manager->setSplitterState("0x14", ui->tableView_CurrentOrder->horizontalHeader()->saveState());

    event->accept();

}

void MainWindow::priceCommitDataOrders(const int value) {

    int cur_row     = ui->tableView_Price->selectionModel()->currentIndex().row();
    QString syn_id  = m_proxy_price->index(cur_row, 1).data().toString();
    QString date    = m_proxy_price->index(cur_row, 11).data().toString();
    double price    = m_proxy_price->index(cur_row, 12).data().toFloat();


    m_helper->countValueChanged(syn_id, value, price, date);

    m_query_model->query().exec();

    ui->tableView_Price->selectRow(cur_row);
    ui->tableView_Price->setFocus();

}

void MainWindow::orderChanged(int order_id, bool show_progrees) {

    if(show_progrees){
        int cur = m_progress_dlg->value();
        m_progress_dlg->setValue(++cur);
    }

    int id_column = 0;
    m_orders_model->select();
    QModelIndex start_ind = m_orders_model->index(0,id_column);
    QModelIndexList lst = m_orders_model->match( start_ind, Qt::DisplayRole,
                                                 order_id, 1, Qt::MatchFixedString);

    int order_row = 0;

    if(!lst.isEmpty())
        order_row = lst.at(0).row();

    selectAllModels(order_row);

}

void MainWindow::operationError(QString title, QString text, bool canceled) {

    if(text.contains("0xDAED")){
       title = "Не активированная копия программы!\t\t";
       text = "Обратитесь в службу поддержки!";
       canceled = true;
       m_isActivated = false;
    }

    QMessageBox *m_box = new QMessageBox(tr("Ошибка\t\t\t\t\t"),title,
                                         QMessageBox::Critical,
                                         QMessageBox::Close,0,0,this);
    m_box->setDetailedText(text);

    m_box->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    m_box->setFixedWidth(350);
    m_box->setBaseSize(350, 50);

    if(canceled){
        m_progress_dlg->cancel();
        delete m_1c_proxy;
    }

    m_box->exec();

}

void MainWindow::on_actionUploadOrders_triggered(){

    UploadDialog *dialog = new UploadDialog(m_helper, this);
    dialog->setWindowModality(Qt::WindowModal);
    dialog->exec();
    selectAllModels();

}

void MainWindow::on_actionCheckUpdate_triggered() {


    QMessageBox::information(this,tr("Поиск обновлений\t\t\t\t\t\t"),tr("Вы используете последнюю версию!"));
    return;

    QProcess *vec = new QProcess;
    QString command = QString("updater.exe inst.exe /S /D= %1 %2").arg(qApp->applicationDirPath())
            .arg(qApp->applicationFilePath());
    vec->startDetached(command);
    qApp->quit();
}

void MainWindow::on_action_Qt_triggered() {
    qApp->aboutQt();
}

void MainWindow::updateAllTables() {

    m_medicines_model->select();

    while (m_medicines_model->canFetchMore())
       m_medicines_model->fetchMore();

    selectAllModels();

    ui->tableView_Medicines->sortByColumn(2);
    ui->tableView_Medicines->setCurrentIndex(m_proxy_medicine->index(0,2));

}

void MainWindow::on_actionAbout_triggered()
{

    QString text = tr("<b> ТредиФарм %1.%2</b> (build:%3)<br><br> Код продукта: %4<br><br><br><i> Данная программа использует Qt версии %5</i>")
            .arg(QString::number(VER_MAJ),
                 QString::number(VER_MIN),
                 QString::number(BUILDN),
                 m_snUID.toString().replace("{","").replace("}", ""),
                 QT_VERSION_STR);

    QMessageBox::about(this, "О ТредиФарм", text);

}

void MainWindow::on_action_Providers_triggered()
{
    ProvidersDialog *dialog = new ProvidersDialog(this, m_manager, &db);

    connect(dialog, SIGNAL(providersListNeedUpdate()), this, SLOT(updateProviders()));
    connect(this, SIGNAL(providersIsUpdated()),dialog, SLOT(updateModel()));

    dialog->setWindowModality(Qt::WindowModal);
    int res = dialog->exec();
    if(res == QDialog::Accepted)
        currentProductChanged(ui->tableView_Medicines->selectionModel()->selection(),
                              ui->tableView_Medicines->selectionModel()->selection());

    dialog->disconnect();
    dialog->deleteLater();

}

void MainWindow::updateProviders(){

    createTransport();
    createDialog(tr("Обновление данных ..."), 10);
    m_progress_dlg->show();
    m_1c_proxy->getProviders();

}

void MainWindow::on_dsb_markups_editingFinished()
{
    currentProductChanged(ui->tableView_Medicines->selectionModel()->selection(),
                          ui->tableView_Medicines->selectionModel()->selection());
}
