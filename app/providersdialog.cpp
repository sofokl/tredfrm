#include "providersdialog.h"
#include "ui_providersdialog.h"

#include <QMessageBox>

ProvidersDialog::ProvidersDialog(QWidget *parent, LkSettingsManager *settings, QSqlDatabase *db) :
    QDialog(parent),
    ui(new Ui::ProvidersDialog)
{
    ui->setupUi(this);

    m_settings = settings;
    connect(ui->checkBox, SIGNAL(toggled(bool)), m_settings, SLOT(setShowAllProvs(bool)));

    m_model = new LkSqlTableModel(this, *db);
    m_model->setTable("Providers");
    m_model->setFilter("is_deleted = 0");

    QSqlError er = m_model->lastError();

    if(er.type() != QSqlError::NoError) {
        QMessageBox::critical(this, tr("Ошибка открытия таблицы"), tr("Ошибка %1\n%2").arg( QString::number(er.type()), er.text()), QMessageBox::Close);
    }

    connect(this, SIGNAL(accepted()), m_model, SLOT(submitAll()));

    initModel();
}

ProvidersDialog::~ProvidersDialog()
{
    delete ui;
}

void ProvidersDialog::on_pushButton_Update_clicked()
{
    emit providersListNeedUpdate();
}

void ProvidersDialog::initModel(){

    m_model->select();
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    //m_model->insertColumn(0);

    //m_model->setHeaderData(0, Qt::Horizontal, tr(""));
    m_model->setHeaderData(0, Qt::Horizontal, tr("Код"));
    m_model->setHeaderData(1, Qt::Horizontal, tr("Поставщик"));
    m_model->setHeaderData(2, Qt::Horizontal, tr("Дата обновления"));
    m_model->setHeaderData(3, Qt::Horizontal, tr("Коэффициент цены"));
    m_model->setHeaderData(4, Qt::Horizontal, tr("Учавствует в анализе"));
    m_model->setHeaderData(5, Qt::Horizontal, tr("Скрыть"));
    m_model->setHeaderData(6, Qt::Horizontal, tr("ПометкаУдаления"));

    m_model->setRowsIcon(QIcon(":/icons/item"), 1);
    m_model->EditableColumns << 3 << 4;
    m_model->ProhibitingColumns << 5 << 6;


    m_filter_model = new QSortFilterProxyModel(this);
    m_filter_model->setSourceModel(m_model);
    m_filter_model->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_filter_model->setDynamicSortFilter(true);
    m_filter_model->setFilterKeyColumn(5);

    ui->tableView->setModel(m_filter_model);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->verticalHeader()->setDefaultSectionSize(20);
    ui->tableView->verticalHeader()->setDefaultAlignment(Qt::AlignVCenter);
    ui->tableView->horizontalHeader()->setMinimumSectionSize(15);
    ui->tableView->horizontalHeader()->setDefaultSectionSize(15);
    ui->tableView->horizontalHeader()->setSectionsMovable(true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->tableView->setEditTriggers(QTableView::DoubleClicked);
    ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableView->sortByColumn(1, Qt::AscendingOrder);
    ui->tableView->setIconSize(QSize(12,12));

    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(5);
    ui->tableView->hideColumn(6);

    coef_delegate = new LkSpinBoxDelegate(this);
    ui->tableView->setItemDelegateForColumn(3, coef_delegate);

    used_delegate = new LkCheckedDelegate(this);
    ui->tableView->setItemDelegateForColumn(4, used_delegate);

    ui->checkBox->setChecked(m_settings->getShowAllProvs());
}

void ProvidersDialog::on_checkBox_toggled(bool checked)
{
    if(checked)
        m_filter_model->setFilterFixedString("0");
    else
        m_filter_model->setFilterFixedString("");

}
void ProvidersDialog::updateModel()
{
    m_model->select();
}
