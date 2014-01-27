#ifndef LKSQLTABLEMODEL_H
#define LKSQLTABLEMODEL_H

#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QAbstractItemModel>
#include <QIcon>
#include <QMap>

#include "core_global.h"

class CORESHARED_EXPORT LkSqlTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    LkSqlTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase()) :
        QSqlTableModel(parent, db)
    {}

    virtual QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const
    {
        QVariant result;
            if (role == Qt::DecorationRole && idx.column() == m_incolumn)
            {
                result = m_icon;
            }
            else if (role == Qt::TextAlignmentRole)
            {
                int al = alignmentColumn[idx.column()];
                if(al == 0)
                    return Qt::AlignLeft;
                else
                    return al;
            }
            else
                result = QSqlTableModel::data(idx, role);

        return result;
    }
    
signals:
    
public slots:
    void setRowsIcon(const QIcon icon, int in_column){
        m_icon = icon;
        m_incolumn = in_column;
    }

    void setColumnAlignment(const int column, const int align) {
        alignmentColumn[column] = align;
    }

private:
    QIcon m_icon;
    int m_incolumn;

    QMap<int, int> alignmentColumn;

};

#endif // LKSQLTABLEMODEL_H
