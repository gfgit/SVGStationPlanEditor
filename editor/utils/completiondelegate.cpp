#include "completiondelegate.h"

#include <QLineEdit>
#include <QCompleter>

CompletionDelegate::CompletionDelegate(QAbstractItemModel *m, QObject *parent) :
    QStyledItemDelegate(parent),
    sourceModel(m)
{

}

QWidget *CompletionDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QLineEdit *ed = new QLineEdit(parent);
    QCompleter *c = new QCompleter(sourceModel, ed);
    c->setCompletionColumn(0);
    c->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    ed->setCompleter(c);
    return ed;
}

void CompletionDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *ed = static_cast<QLineEdit *>(editor);
    ed->setText(index.data(Qt::DisplayRole).toString());
}

void CompletionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *ed = static_cast<QLineEdit *>(editor);
    model->setData(index, ed->text(), Qt::DisplayRole);
}
