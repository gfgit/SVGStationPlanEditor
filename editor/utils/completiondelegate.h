#ifndef COMPLETIONDELEGATE_H
#define COMPLETIONDELEGATE_H

#include <QStyledItemDelegate>

class CompletionDelegate : public QStyledItemDelegate
{
public:
    CompletionDelegate(QAbstractItemModel *m, QObject *parent = nullptr);

    // editing
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;

private:
    QAbstractItemModel *sourceModel;
};

#endif // COMPLETIONDELEGATE_H
