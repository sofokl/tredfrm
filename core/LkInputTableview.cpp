#include "LkInputTableView.h"
#include <QAction>
#include <QHeaderView>

LkInputTableView::LkInputTableView(QWidget *parent) : QTableView(parent)
{
    m_dialog = new QInputDialog(this);
    m_dialog->setInputMode(QInputDialog::IntInput);
    m_dialog->setIntRange(0, 99999);
    m_dialog->setLabelText("Количество к заказу:");
    m_dialog->setCancelButtonText(tr("Отмена"));

    connect(m_dialog->findChild<QSpinBox*>(), SIGNAL(valueChanged(int)), this, SLOT(onSpinBoxValueChanged()), Qt::QueuedConnection);
    connect(m_dialog, SIGNAL(finished(int)), this, SLOT(inputFinished(int)));

}

void LkInputTableView::onSpinBoxValueChanged(){

    sender()->findChild<QLineEdit*>()->deselect();
}

void LkInputTableView::inputFinished(int result){

    if(result == QInputDialog::Accepted) {
        int value = m_dialog->intValue();
        m_dialog->setIntValue(0);
        emit editFinished(value);
    }

}

void LkInputTableView::keyboardSearch(const QString &search)
{

    if( this->selectedIndexes().isEmpty()){
        return;
    }

    QString name = QString("%1\t\t\t").arg(this->model()->index(currentIndex().row(), 6).data().toString());
    m_dialog->setWindowTitle(name);
    m_dialog->show();
    m_dialog->setIntValue(search.toInt());

}

void LkInputTableView::setModel(QAbstractItemModel *m_model){

    QTableView::setModel(m_model);

    this->setContextMenuPolicy(Qt::ActionsContextMenu);
    this->actions().clear();

    for(int i=0; i < this->model()->columnCount(); i++){
        int data = this->model()->headerData(i, Qt::Horizontal, Qt::UserRole).toInt();
        if(data == hideColumnFlag)
            QTableView::hideColumn(i);
        else {
            QString text = this->model()->headerData(i, Qt::Horizontal).toString();
            QAction *openAct = new QAction(text, this);
            openAct->setCheckable(true);
            openAct->setChecked(!isColumnHidden(i));
            openAct->setData(i);
            openAct->setStatusTip(text);
            connect(openAct, SIGNAL(triggered(bool)), this, SLOT(on_action(bool)));
            this->addAction(openAct);
        }
    }
}

void LkInputTableView::on_action(bool trigered){

   QObject* obj=QObject::sender();
   if (QAction *act = qobject_cast<QAction *>(obj)){
       int column = act->data().toInt();
       QTableView::setColumnHidden(column, !trigered);
   }
}

//void LkInputTableView::setColumnHidden(int column, bool hiden) {
//    qDebug() << column << hiden;
//    //QTableView::hideColumn(column);
//}
