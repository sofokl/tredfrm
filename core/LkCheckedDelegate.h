#ifndef LKCHECKEDDELEGATE_H
#define LKCHECKEDDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QCheckBox>
#include <QApplication>
#include <QtGui/QMouseEvent>

#include "core_global.h"

class CORESHARED_EXPORT LkCheckedDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:

    /*explicit LkCheckedDelegate(QObject *parent = 0) : QStyledItemDelegate(parent){}

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
    
//    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
//                          const QModelIndex &index) const {

//        QCheckBox *editor = new QCheckBox(parent);
//        editor->setText("");
//        //editor->setFrame(false);
//       // editor->setMinimum(0);
//       // editor->setMaximum(99999);
//       // editor->setDecimals(2);
//       // editor->setButtonSymbols(QDoubleSpinBox::PlusMinus);

//        return editor;
//    }
signals:
    
public slots:
  */

//#include "check_box_delegate.h"

//#include <QtGui/QApplication>

static QRect CheckBoxRect(const QStyleOptionViewItem &view_item_style_options) {
    QStyleOptionButton check_box_style_option;
    QRect check_box_rect = QApplication::style()->subElementRect(
                QStyle::SE_CheckBoxIndicator,
                &check_box_style_option);
    QPoint check_box_point(view_item_style_options.rect.x() +
                           view_item_style_options.rect.width() / 2 -
                           check_box_rect.width() / 2,
                           view_item_style_options.rect.y() +
                           view_item_style_options.rect.height() / 2 -
                           check_box_rect.height() / 2);
    return QRect(check_box_point, check_box_rect.size());
}

explicit LkCheckedDelegate(QObject *parent)  : QStyledItemDelegate(parent) {
}

void LkCheckedDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
  bool checked = index.model()->data(index, Qt::DisplayRole).toBool();

  QStyleOptionButton check_box_style_option;
  check_box_style_option.state |= QStyle::State_Enabled;
  if (checked) {
    check_box_style_option.state |= QStyle::State_On;
  } else {
    check_box_style_option.state |= QStyle::State_Off;
  }
  check_box_style_option.rect = CheckBoxRect(option);

  QApplication::style()->drawControl(QStyle::CE_CheckBox,
                                     &check_box_style_option,
                                     painter);
}

// This is essentially copied from QStyledItemEditor, except that we
// have to determine our own "hot zone" for the mouse click.
bool LkCheckedDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
  if ((event->type() == QEvent::MouseButtonRelease) ||
      (event->type() == QEvent::MouseButtonDblClick)) {
    QMouseEvent *mouse_event = static_cast<QMouseEvent*>(event);
    if (mouse_event->button() != Qt::LeftButton ||
        !CheckBoxRect(option).contains(mouse_event->pos())) {
      return false;
    }
    if (event->type() == QEvent::MouseButtonDblClick) {
      return true;
    }
  } else if (event->type() == QEvent::KeyPress) {
    if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space &&
        static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select) {
      return false;
    }
  } else {
    return false;
  }

  bool checked = index.model()->data(index, Qt::DisplayRole).toBool();
  return model->setData(index, !checked, Qt::EditRole);
}


};

#endif // LKCHECKEDDELEGATE_H
