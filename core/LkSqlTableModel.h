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
    {

    }

    virtual QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const
    {
        QVariant result;;

        switch(role)
        {
        case Qt::CheckStateRole: {
            if(CheckableColumns.contains(idx.column())) {
                QModelIndex index_id = index(idx.row(), idx.column());
                bool value = QSqlTableModel::data(index_id, Qt::DisplayRole).toBool();
                result = value ? Qt::Checked : Qt::Unchecked;
            }
            break;
        }
        case Qt::TextAlignmentRole: {
                int al = alignmentColumn[idx.column()];
                if(al == 0)
                    result = Qt::AlignLeft;
                else
                    result = al;
            }
            break;

        case Qt::TextColorRole: {
            QModelIndex index_id = index(idx.row(), m_colorColumn);
            bool value = index_id.data().toBool();
                if(value)
                    result = m_textColor;
                break;
            }
        case Qt::DecorationRole: {
            if(idx.column() == m_incolumn)
                 result = m_icon;
            break;
        }

        case Qt::DisplayRole: {
            if(CheckableColumns.contains(idx.column())){
                result = "";
            }
            break;
        }

        case Qt::EditRole: {
            if(CheckableColumns.contains(idx.column())){
                QModelIndex index_id = index(idx.row(), idx.column());
                result = QSqlTableModel::data(index_id, Qt::DisplayRole).toBool();
            }
            break;
        }

        default:
            break;
        }

        if(!result.isValid())
            result = QSqlTableModel::data(idx, role);

        return result;
    }
    
    virtual Qt::ItemFlags LkSqlTableModel::flags(const QModelIndex &idx) const {

        for (int i = 0; i < ProhibitingColumns.size(); ++i) {
            int x_column = ProhibitingColumns.at(i);
            QModelIndex index_id = index( idx.row(), x_column);
            if (index_id.data().toBool())
               return Qt::ItemIsSelectable;
        }

       Qt::ItemFlags result =  Qt::ItemIsEnabled | Qt::ItemIsSelectable;

       if (EditableColumns.contains(idx.column()))
             result |= Qt::ItemIsEditable;

       if (CheckableColumns.contains(idx.column()))
            result  |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable;

        return result;
    }

    virtual bool LkSqlTableModel::setData( const QModelIndex & index, const QVariant & value, int role)
    {
        if (index.isValid() && CheckableColumns.contains(index.column()) && role==Qt::CheckStateRole)
        {
           bool chk = (value.toInt()==Qt::Checked);
           return QSqlTableModel::setData(index, chk);
        }else
           return QSqlTableModel::setData(index,value,role);
    }

    QList<int> EditableColumns;
    QList<int> CheckableColumns;
    QList<int> ProhibitingColumns;

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

#endif // LKSQLTABLEMODEL_H
