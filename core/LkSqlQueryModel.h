#ifndef LKSQLQUERYMODEL_H
#define LKSQLQUERYMODEL_H

#include <QSqlQuery>
#include <QSqlRelationalTableModel>
#include <QAbstractItemModel>
#include <QIcon>
#include <QMap>
#include <QBrush>

#include "core_global.h"

class CORESHARED_EXPORT LkSqlQueryModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    LkSqlQueryModel(QObject *parent = 0) :
        QSqlQueryModel(parent)
    {

        m_alternateIconFlagSource = -1;
    }

    virtual QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const
    {
            QVariant result;
            if (role == Qt::DecorationRole && idx.column() == m_incolumn)
            {
                result = m_icon;
            }
            else
            if (role == Qt::TextAlignmentRole)
            {
                int al = alignmentColumn[idx.column()];
                if(al == 0)
                    result = Qt::AlignLeft;
                else
                    result = al;
            }
            else if (role == Qt::TextColorRole)
            {
                QModelIndex index_id = index(idx.row(), m_colorColumn);
                bool isLife = index_id.data().toBool();
                    if(isLife)
                        result = m_textColor;
               //return result;
            }
            else if (role == Qt::BackgroundRole) {
                if(idx.column() == m_alternateBackgroundColumn){
                    QModelIndex index_id = index(idx.row(), m_alternateBackgroundSource);
                    bool flag = index_id.data().toBool();
                    if(flag)
                        result = m_alternateBrush;
                }

            }

            if(!result.isValid())
                result = QSqlQueryModel::data(idx, role);

        return result;
    }

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        if(orientation == Qt::Vertical)
        {
            if(role == Qt::DecorationRole){
                if(m_alternateIconFlagSource >= 0){
                    QModelIndex index_id = index(section, m_alternateIconFlagSource);
                    bool flag = index_id.data().toBool();
                    if(flag)
                        return m_alt_header_icon;
                    else
                        return m_header_icon;
                }
                return &m_header_icon;
            }
            if(role == Qt::DisplayRole)
                return "";
        }

        return QSqlQueryModel::headerData(section, orientation, role);
    }

signals:

public slots:

    void setRowsIcon(const QIcon icon, int in_column=-1){
        m_icon = icon;
        m_incolumn = in_column;
    }

    void exec(QList<QVariant> list) {

        query().prepare(query().executedQuery());

        for (int i = 0; i < list.size(); ++i)
                query().bindValue(i,list.at(i));

        query().exec();
        setQuery(query());
    }

    void exec(int countValues, QVariant value) {

        query().prepare(query().executedQuery());

        for (int i = 0; i < countValues; ++i)
                query().bindValue(i, value);

        query().exec();
        setQuery(query());
    }

    void setColumnAlignment(const int column, const int align) {
        alignmentColumn[column] = align;
    }

    void setColorTextByValueColumn(const int column, const QColor color) {
        m_colorColumn = column;
        m_textColor = color;
    }

    void setVerticalHeadersIcon(QIcon icon, QIcon alt_icon){
        m_header_icon = icon;
        m_alt_header_icon = alt_icon;
    }

    void setAlternateIconColumnSource(int columnIndex){
        m_alternateIconFlagSource = columnIndex;
    }

    void setAlternatebackgroundBySource(int destColumn, int sourceColumn, QBrush brush){
        m_alternateBackgroundColumn = destColumn;
        m_alternateBackgroundSource = sourceColumn;
        m_alternateBrush = brush;
    }

private:
    QIcon m_icon;
    int m_incolumn;
    int m_colorColumn;

    QIcon m_header_icon;
    QIcon m_alt_header_icon;
    int m_alternateIconFlagSource;

    QColor m_textColor;
    QMap<int, int> alignmentColumn;
    int m_alternateBackgroundColumn;
    int m_alternateBackgroundSource;
    QBrush m_alternateBrush;



};

#endif // LKSQLQUERYMODEL_H
