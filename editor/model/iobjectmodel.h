#ifndef IOBJECTMODEL_H
#define IOBJECTMODEL_H

#include <QAbstractTableModel>

namespace ssplib {
class ItemBase;
class ElementPath;
} // namespace ssplib

class IObjectModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit IObjectModel(QObject *parent = nullptr);

    virtual bool addItem();
    virtual bool editItem(int row);
    virtual bool removeItem(int row);

    virtual void clear();

    virtual bool addElementToItem(ssplib::ElementPath &p, ssplib::ItemBase *item);
    virtual bool removeElementFromItem(ssplib::ItemBase *item, int pos);

    virtual const ssplib::ItemBase* getItemAt(int row);
    virtual int getItemCount() const;

signals:
    void itemRemoved(int row);
};

#endif // IOBJECTMODEL_H
