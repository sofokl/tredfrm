#ifndef LKCHECKEDDELEGATE_H
#define LKCHECKEDDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QCheckBox>
#include <QApplication>

#include "core_global.h"

class CORESHARED_EXPORT LkCheckedDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit LkCheckedDelegate(QObject *parent = 0) : QStyledItemDelegate(parent){}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

        QString value =  index.model()->data(index, Qt::DisplayRole).toString();

        bool checked = false;

        if(value == "Y" || value == "1") checked = true;

        QStyleOptionButton checkboxstyle;
        QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxstyle);
        checkboxstyle.rect = option.rect;
        checkboxstyle.rect.setLeft(option.rect.x() + option.rect.width()/2 - checkbox_rect.width()/2);

        if(checked) checkboxstyle.state = QStyle::State_On|QStyle::State_Enabled;
        else checkboxstyle.state = QStyle::State_Off|QStyle::State_Enabled;

        QStyledItemDelegate::paint(painter, option, index);

        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkboxstyle, painter);
    }

    virtual QString displayText ( const QVariant &, const QLocale &) const {


        return QString();
    }
    
signals:
    
public slots:
    
};

#endif // LKCHECKEDDELEGATE_H
