#ifndef LKINPUTTABLEVIEW_H
#define LKINPUTTABLEVIEW_H

#include <QTableView>
#include <QSpinBox>
#include <QAbstractItemView>
#include <QModelIndex>
#include <QInputDialog>

#include "core_global.h"

class CORESHARED_EXPORT LkInputTableView : public QTableView
{
    Q_OBJECT
public:
    explicit LkInputTableView(QWidget *parent = 0);
    virtual void keyboardSearch(const QString &search);
    virtual void setModel(QAbstractItemModel *model);
    static const int hideColumnFlag = 0x2;


public slots:
   // virtual void hideColumn(int column);

signals:
    void editFinished(int);

private:
    QInputDialog *m_dialog;

private slots:
    void onSpinBoxValueChanged();
    void inputFinished(int result);
    void on_action(bool trigered);


    Q_DECLARE_PRIVATE(QTableView)

};

#endif // LKINPUTTABLEVIEW_H
