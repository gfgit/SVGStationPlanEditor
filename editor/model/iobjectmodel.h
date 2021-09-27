#ifndef IOBJECTMODEL_H
#define IOBJECTMODEL_H

#include <QAbstractTableModel>


class ItemBase;
class ElementPath;

class IObjectModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit IObjectModel(QObject *parent = nullptr);

    virtual bool addItem();
    virtual bool editItem(int row);
    virtual bool removeItem(int row);

    virtual void clear();

    virtual bool addElementToItem(ElementPath &p, ItemBase *item);
    virtual bool removeElementFromItem(ItemBase *item, int pos);

    virtual const ItemBase* getItemAt(int row);
    virtual int getItemCount() const;

signals:
    void itemRemoved(int row);
};

#endif // IOBJECTMODEL_H
