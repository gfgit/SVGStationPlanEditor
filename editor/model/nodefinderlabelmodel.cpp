#include "nodefinderlabelmodel.h"

#include "editor/manager/nodefindermgr.h"

NodeFinderLabelModel::NodeFinderLabelModel(NodeFinderMgr *mgr, QObject *parent) :
    IObjectModel(parent),
    nodeMgr(mgr)
{
}

QVariant NodeFinderLabelModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case LabelNameCol:
            return tr("Label Name");
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int NodeFinderLabelModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : items.size();
}

int NodeFinderLabelModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : NCols;
}

QVariant NodeFinderLabelModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    const LabelItem& item = items.at(idx.row());

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case LabelNameCol:
            return item.gateLetter;
        }
        break;
    }
    case Qt::CheckStateRole:
    {
        switch (idx.column())
        {
        case LabelNameCol:
            return item.visible ? Qt::Checked : Qt::Unchecked;
        }
        break;
    }
    }

    return QVariant();
}

bool NodeFinderLabelModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (!idx.isValid())
        return false;

    LabelItem& item = items[idx.row()];

    switch (role)
    {
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case LabelNameCol:
            const QString name = value.toString().simplified();
            if(name.isEmpty() || name.front() < 'A' || name.front() > 'Z')
                return false;

            //Set name
            item.gateLetter = name.front();
        }
        break;
    }
    case Qt::CheckStateRole:
    {
        switch (idx.column())
        {
        case LabelNameCol:
        {
            Qt::CheckState cs = value.value<Qt::CheckState>();
            item.visible = cs == Qt::Checked;
        }
        }
        break;
    }
    }

    std::sort(items.begin(), items.end());

    emit dataChanged(idx, idx);
    emit nodeMgr->repaintSVG();

    return true;
}

Qt::ItemFlags NodeFinderLabelModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f;
    if(!idx.isValid())
        return f;

    f.setFlag(Qt::ItemIsEnabled);
    f.setFlag(Qt::ItemIsSelectable);
    f.setFlag(Qt::ItemIsEditable);
    f.setFlag(Qt::ItemIsUserCheckable);
    return f;
}

void NodeFinderLabelModel::setItems(const QVector<LabelItem> &vec)
{
    beginResetModel();
    items = vec;
    endResetModel();
}

void NodeFinderLabelModel::clear()
{
    beginResetModel();
    items.clear();
    items.squeeze();
    endResetModel();
}

bool NodeFinderLabelModel::addElementToItem(ElementPath &p, ItemBase *item)
{
    if(item < items.data() || item >= items.data() + items.size() || item->elements.contains(p))
        return false; //Not a label item

    LabelItem *ptr = static_cast<LabelItem *>(item);
    int row = ptr - items.data(); //Pointer aritmetics

    p.elem.setAttribute(svg_attr::LabelName, ptr->gateLetter);
    item->elements.append(p);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);

    return true;
}

bool NodeFinderLabelModel::removeElementFromItem(ItemBase *item, int pos)
{
    if(item < items.data() || item >= items.data() + items.size())
        return false; //Not a label item

    LabelItem *ptr = static_cast<LabelItem *>(item);
    int row = ptr - items.data(); //Pointer aritmetics

    ptr->elements[pos].elem.removeAttribute(svg_attr::LabelName);
    ptr->elements.removeAt(pos);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);

    return true;
}

const ItemBase* NodeFinderLabelModel::getItemAt(int row)
{
    if(row < 0 || row >= items.size())
        return nullptr;

    return &items.at(row);
}

int NodeFinderLabelModel::getItemCount() const
{
    return items.size();
}

bool NodeFinderLabelModel::addItem()
{
    nodeMgr->clearCurrentItem();

    LabelItem item;
    item.gateLetter = '-';
    item.visible = false;

    beginInsertRows(QModelIndex(), items.size(), items.size());
    items.append(item);
    endInsertRows();

    return true;
}

bool NodeFinderLabelModel::removeItem(int row)
{
    if(row < 0 || row >= items.size())
        return false;

    nodeMgr->clearCurrentItem();

    ItemBase& item = items[row];

    for(ElementPath& elemPath : item.elements)
    {
        elemPath.elem.removeAttribute(svg_attr::LabelName);
    }

    beginRemoveRows(QModelIndex(), row, row);
    items.removeAt(row);
    endRemoveRows();

    return true;
}

bool NodeFinderLabelModel::editItem(int row)
{
    if(row < 0 || row >= items.size())
        return false;

    ItemBase *item = &items[row];
    nodeMgr->requestEditItem(item, EditingModes::LabelEditing);

    return true;
}
