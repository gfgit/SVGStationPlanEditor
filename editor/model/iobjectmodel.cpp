#include "iobjectmodel.h"

IObjectModel::IObjectModel(QObject *parent) :
    QAbstractTableModel(parent)
{

}

bool IObjectModel::addItem()
{
    return false;
}

bool IObjectModel::editItem(int row)
{
    return false;
}

bool IObjectModel::removeItem(int row)
{
    return false;
}

void IObjectModel::clear()
{

}

bool IObjectModel::addElementToItem(ElementPath &p, ItemBase *item)
{
    return false;
}

bool IObjectModel::removeElementFromItem(ItemBase *item, int pos)
{
    return false;
}
