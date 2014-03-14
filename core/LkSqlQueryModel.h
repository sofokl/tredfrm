#ifndef LKSQLQUERYMODEL_H
#define LKSQLQUERYMODEL_H

#include <QSqlQuery>
#include <QSqlRelationalTableModel>
#include <QAbstractItemModel>
#include <QIcon>
#include <QMap>

#include "core_global.h"

class CORESHARED_EXPORT LkSqlQueryModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    LkSqlQueryModel(QObject *parent = 0) :
        QSqlQueryModel(parent)
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
            else if (role == Qt::TextColorRole)
            {
                QModelIndex index_id = index(idx.row(), m_colorColumn);
                bool isLife = index_id.data().toBool();
                    if(isLife)
                        result = m_textColor;
               return result;
            }
            else
                result = QSqlQueryModel::data(idx, role);

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

    void setColorTextByValueColumn(const int column, const QColor color) {
        m_colorColumn = column;
        m_textColor = color;
    }

private:
    QIcon m_icon;
    int m_incolumn;
    int m_colorColumn;
    QColor m_textColor;

    QMap<int, int> alignmentColumn;

};

#endif // LKSQLQUERYMODEL_H
