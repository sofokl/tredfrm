#include "uploaddialog.h"
#include "ui_uploaddialog.h"
#include <QFile>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>

UploadDialog::UploadDialog(LkStoreHelper *helper, QWidget *parent) : QDialog(parent), ui(new Ui::UploadDialog)
{
    ui->setupUi(this);
    m_helper = helper;
    setProvidersList();
    setSalepointList();
}

UploadDialog::~UploadDialog()
{
    delete ui;
}

void UploadDialog::on_tbSelectFile_clicked()
{
    QFileDialog dialog(this, tr("Выберите файл для загрузки"), "", "Текстовый документ (*.csv *.txt);;" );
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (dialog.exec()){
        ui->leFileName->setText(dialog.selectedFiles().at(0));
    }
}

void UploadDialog::setProvidersList()
{
    QHash<QString, QString> data = m_helper->getProviders();

    foreach (QString key, data.keys())
    {
        ui->cbProvider->addItem(data.value(key), key);
    }

}

void UploadDialog::setSalepointList()
{
    QHash<QString, QString> data = m_helper->getSalepoints();
    foreach (QString key, data.keys())
    {
        ui->cbSalepoints->addItem(data.value(key), key);
    }

}

void UploadDialog::on_pushButton_clicked()
{
    ui->textBrowser->clear();

    if(ui->leFileName->text().isEmpty()){
        QMessageBox::warning(this, tr(""), tr("Не выбран файл для загрузки!"));
        return;
    }

    if(ui->cbProvider->currentIndex() == -1){
        QMessageBox::warning(this, tr(""), tr("Не выбран поставщик!"));
        return;
    }

    if(ui->cbSalepoints->currentIndex() == -1){
        QMessageBox::warning(this, tr(""), tr("Не выбран пункт доставки!"));
        return;
    }

    if(ui->cbSplitter->currentIndex() == -1){
        QMessageBox::warning(this, tr(""), tr("Не выбран разделитель текста!"));
        return;
    }

    QFile file(ui->leFileName->text());
    if(!file.open(QFile::ReadOnly)){
        QMessageBox::warning(this, tr("Не удается прочитать файл               "),
                             file.errorString());
        return;
    }

    QTextStream data(&file);
    data.setCodec(QTextCodec::codecForName("CP1251"));

    QString splitter;
    switch(ui->cbSplitter->currentIndex())
    {
    case 0:
        splitter = '\t';break;
    case 1:
        splitter = "|"; break;
    case 2:
        splitter = ";"; break;
    default:
        break;
    }

   // QHash<int, QMap<QString, QVariant> > upload_data;

    QString prov_code = ui->cbProvider->itemData(ui->cbProvider->currentIndex()).toString();
    QString sale_code = ui->cbSalepoints->itemData(ui->cbSalepoints->currentIndex()).toString();

    int counter = 0;
    while(!data.atEnd()){
        counter++;
        if(counter == 1 && ui->checkHeader->isChecked()){
            data.readLine();
            counter++;
        }

        QStringList row = data.readLine().replace("\"", "").split(splitter);
        if(row.count() < 2){
            QString text = ("<font color='red'>[Строка №%1] Ошибка разделения строки</font><br>");

            ui->textBrowser->insertHtml(text.arg(counter));
            continue;
        }

        TData data_row;
        data_row["prov_code"] = prov_code;
        data_row["sale_code"]= sale_code;
        data_row["syn_code"] = row.at(0).trimmed();
        data_row["count"] = row.at(1).trimmed();

        QString err = m_helper->addToOrderByCode(data_row);

        if(!err.trimmed().isEmpty()){
            QString text = ("<font color='red'>[Строка №%1] %2</font><br>");

            ui->textBrowser->insertHtml(text.arg(QString::number(counter), err));
        }

    }

    file.close();

    if(counter >= 1)
        ui->textBrowser->insertHtml("<font color='blue'>Загрузка завершена</font><br>");

}
