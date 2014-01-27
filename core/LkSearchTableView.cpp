#include "LkSearchTableView.h"

LkSearchTableView::LkSearchTableView(QWidget *parent) : QTableView(parent)
{
    m_searchLabel = new QLineEdit(this);
    m_searchLabel->setStyleSheet( "QLineEdit { background-color : white; color : blue; border-bottom-color: blue; font: italic; }" );
    m_searchLabel->setVisible(false);
    m_searchLabel->setPlaceholderText("Введите значение для поиска");
    m_column = -1;
}



void LkSearchTableView::keyPressEvent(QKeyEvent * event)
{
    if( event->key() == Qt::Key_Escape )
    {
        m_searchLabel->setText("");
        if( m_searchLabel->isVisible() )
        {
            m_searchLabel->setVisible(false);

        }
        m_column = -1;
    }
    else if (event->key() == Qt::Key_Backspace && m_searchLabel->isVisible()) {
        keyboardSearch("\b");
    }
    QTableView::keyPressEvent(event);
}

void LkSearchTableView::keyboardSearch(const QString &search)
{
    if( this->selectedIndexes().isEmpty()) return;

    int search_col = this->currentIndex().column();

    if( search_col < 0 ) return;

    if(m_column != -1)
        search_col = m_column;
    else
        m_column = search_col;

    QModelIndex start_ind = this->model()->index(0, search_col);

    if( !m_searchLabel->isVisible() )
    {
        m_searchLabel->setGeometry(5,this->height()-20,this->width()-25,20);
        m_searchLabel->show();
        this->setFocus();
    }

    QString search_string = m_searchLabel->text();

    if( search.at(0) == '\b' )
    {
        int l = search_string.length();
        if( l > 0 ) search_string = search_string.left(l-1);
    }
    else
        search_string += search;

    QModelIndexList lst = model()->match(start_ind, Qt::DisplayRole, search_string, 1, Qt::MatchStartsWith );

    if( lst.isEmpty() ) {
        m_searchLabel->setPlaceholderText("Ничего не найдено");
        return;
    } else {
        m_searchLabel->setPlaceholderText("Введите значение для поиска");
    }

    m_searchLabel->setText(search_string);

    this->setCurrentIndex(lst.first());
}
