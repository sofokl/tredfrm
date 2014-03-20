#ifndef LKSEARCHTABLEVIEW_H
#define LKSEARCHTABLEVIEW_H

#include <QTableView>
#include <QLineEdit>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QAbstractItemView>

#include "core_global.h"

class CORESHARED_EXPORT LkSearchTableView : public QTableView
{
    Q_OBJECT
public:
    explicit LkSearchTableView(QWidget *parent = 0);
    virtual void keyPressEvent(QKeyEvent * event);
    virtual void keyboardSearch(const QString &search);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void setModel(QAbstractItemModel *m_model);
    
signals:
    
private:
    QLineEdit *m_searchLabel;
    int m_column;

    //Q_DECLARE_PRIVATE(QTableView)
};

#endif // LKSEARCHTABLEVIEW_H
