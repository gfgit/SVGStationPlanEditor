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

bool IObjectModel::addElementToItem(ssplib::ElementPath &p, ssplib::ItemBase *item)
{
    return false;
}

bool IObjectModel::removeElementFromItem(ssplib::ItemBase *item, int pos)
{
    return false;
}
