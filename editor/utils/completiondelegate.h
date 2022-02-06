#ifndef COMPLETIONDELEGATE_H
#define COMPLETIONDELEGATE_H

#include <QStyledItemDelegate>

class ComplEdit;

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

    int getColumn() const;
    void setColumn(int newColumn);

    int getRole() const;
    void setRole(int newRole);

public slots:
    void onActivated(ComplEdit *ed, const QModelIndex& idx);

private:
    QAbstractItemModel *sourceModel;
    int column;
    int role;
};

#endif // COMPLETIONDELEGATE_H
