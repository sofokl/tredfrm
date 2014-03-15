#include "LkInputTableView.h"

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
        //m_dialog->setIntValue(0);
        return;
    }

    QString name = QString("%1\t\t\t").arg(this->model()->index(currentIndex().row(), 6).data().toString());
    m_dialog->setWindowTitle(name);
    m_dialog->show();
    m_dialog->setIntValue(search.toInt());

}
