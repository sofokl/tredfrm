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

signals:
    void editFinished(int);

private:
    QInputDialog *m_dialog;

private slots:
    void onSpinBoxValueChanged();
    void inputFinished(int result);

    Q_DECLARE_PRIVATE(QTableView)

};

#endif // LKINPUTTABLEVIEW_H
