#include "comboboxdelegate.h"

#include <QComboBox>

ComboboxDelegate::ComboboxDelegate(const QStringList& list, QObject *parent) :
    QStyledItemDelegate(parent)
{
    m_list = list;
}

QWidget *ComboboxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *box = new QComboBox(parent);
    box->addItems(m_list);
    return box;
}

void ComboboxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *box = static_cast<QComboBox *>(editor);
    int val = index.data(Qt::EditRole).toInt();
    box->setCurrentIndex(val);
}

void ComboboxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *box = static_cast<QComboBox *>(editor);
    model->setData(index, box->currentIndex(), Qt::EditRole);
}
