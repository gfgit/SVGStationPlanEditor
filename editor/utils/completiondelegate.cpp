#include "completiondelegate.h"

#include <QLineEdit>
#include <QCompleter>

class ComplEdit : public QLineEdit
{
public:
    ComplEdit(QWidget *p) : QLineEdit(p) {}

    QModelIndex lastActivatedIdx;
};

CompletionDelegate::CompletionDelegate(QAbstractItemModel *m, QObject *parent) :
    QStyledItemDelegate(parent),
    sourceModel(m),
    column(0),
    role(Qt::EditRole)
{

}

QWidget *CompletionDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    ComplEdit *ed = new ComplEdit(parent);
    QCompleter *c = new QCompleter(sourceModel, ed);
    c->setCompletionColumn(column);
    c->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    ed->setCompleter(c);

    connect(c, qOverload<const QModelIndex &>(&QCompleter::activated), ed,
            [this, ed](const QModelIndex& idx)
            {
                auto self = const_cast<CompletionDelegate *>(this);
                self->onActivated(ed, idx);
            });

    return ed;
}

void CompletionDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    ComplEdit *ed = static_cast<ComplEdit *>(editor);
    ed->setText(index.data(Qt::DisplayRole).toString());
    ed->completer()->complete(QRect());
}

void CompletionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    ComplEdit *ed = static_cast<ComplEdit *>(editor);
    QVariant val = sourceModel->data(ed->lastActivatedIdx, role);
    model->setData(index, val, role);
}

int CompletionDelegate::getColumn() const
{
    return column;
}

void CompletionDelegate::setColumn(int newColumn)
{
    column = newColumn;
}

int CompletionDelegate::getRole() const
{
    return role;
}

void CompletionDelegate::setRole(int newRole)
{
    role = newRole;
}

void CompletionDelegate::onActivated(ComplEdit *ed, const QModelIndex &idx)
{
    ed->lastActivatedIdx = idx;
    emit commitData(ed);
    emit closeEditor(ed);
}
