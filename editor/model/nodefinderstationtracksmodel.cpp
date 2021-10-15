#include "nodefinderstationtracksmodel.h"

#include "manager/nodefindermgr.h"

#include "ssplib/utils/svg_path_utils.h"

#include <QBrush>

NodeFinderStationTracksModel::NodeFinderStationTracksModel(NodeFinderMgr *mgr, QObject *parent) :
    IObjectModel(parent),
    nodeMgr(mgr)
{
}

QVariant NodeFinderStationTracksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case TrackNameCol:
            return tr("Track Name");
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int NodeFinderStationTracksModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : items.size();
}

int NodeFinderStationTracksModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : NCols;
}

QVariant NodeFinderStationTracksModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    const ssplib::TrackItem& item = items.at(idx.row());

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case TrackNameCol:
            return item.trackPos;
        }
        break;
    }
    case Qt::BackgroundRole:
    {
        switch (idx.column())
        {
        case TrackNameCol:
            if(item.elements.isEmpty())
                return QBrush(Qt::yellow);
            break;
        }
        break;
    }
    case Qt::CheckStateRole:
    {
        switch (idx.column())
        {
        case TrackNameCol:
            return item.visible ? Qt::Checked : Qt::Unchecked;
        }
        break;
    }
    }

    return QVariant();
}

bool NodeFinderStationTracksModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (!idx.isValid())
        return false;

    ssplib::TrackItem& item = items[idx.row()];

    switch (role)
    {
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case TrackNameCol:
        {
            bool ok = false;
            int trk = value.toInt(&ok);
            if(!ok || trk < 0)
                return false;

            //Set name
            item.trackPos = trk;
            break;
        }
        }
        break;
    }
    case Qt::CheckStateRole:
    {
        switch (idx.column())
        {
        case TrackNameCol:
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

Qt::ItemFlags NodeFinderStationTracksModel::flags(const QModelIndex &idx) const
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

void NodeFinderStationTracksModel::setItems(const QVector<ssplib::TrackItem> &vec)
{
    beginResetModel();
    items = vec;
    endResetModel();
}

void NodeFinderStationTracksModel::clear()
{
    beginResetModel();
    items.clear();
    items.squeeze();
    endResetModel();
}

bool NodeFinderStationTracksModel::addElementToItem(ssplib::ElementPath &p, ssplib::ItemBase *item)
{
    if(item < items.data() || item >= items.data() + items.size() || item->elements.contains(p))
        return false; //Not a label item

    ssplib::TrackItem *ptr = static_cast<ssplib::TrackItem *>(item);
    int row = ptr - items.data(); //Pointer aritmetics

    p.elem.setAttribute(ssplib::svg_attr::TrackPos, QString::number(ptr->trackPos));
    item->elements.append(p);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);

    return true;
}

bool NodeFinderStationTracksModel::removeElementFromItem(ssplib::ItemBase *item, int pos)
{
    if(item < items.data() || item >= items.data() + items.size())
        return false; //Not a label item

    ssplib::TrackItem *ptr = static_cast<ssplib::TrackItem *>(item);
    int row = ptr - items.data(); //Pointer aritmetics

    clearElement(ptr->elements[pos]);
    ptr->elements.removeAt(pos);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);

    return true;
}

bool NodeFinderStationTracksModel::addItem()
{
    nodeMgr->clearCurrentItem();

    int maxTrackPos = -1;
    for(const ssplib::TrackItem& track : qAsConst(items))
    {
        if(track.trackPos > maxTrackPos)
            maxTrackPos = track.trackPos;
    }

    ssplib::TrackItem item;
    item.trackPos = maxTrackPos + 1;
    item.visible = false;

    beginInsertRows(QModelIndex(), items.size(), items.size());
    items.append(item);
    endInsertRows();

    return true;
}

void NodeFinderStationTracksModel::clearElement(ssplib::ElementPath &elemPath)
{
    elemPath.elem.removeAttribute(ssplib::svg_attr::TrackPos);
}

bool NodeFinderStationTracksModel::removeItem(int row)
{
    if(row < 0 || row >= items.size())
        return false;

    nodeMgr->clearCurrentItem();

    ssplib::ItemBase& item = items[row];

    for(ssplib::ElementPath& elemPath : item.elements)
        clearElement(elemPath);

    beginRemoveRows(QModelIndex(), row, row);
    items.removeAt(row);
    endRemoveRows();

    return true;
}

bool NodeFinderStationTracksModel::editItem(int row)
{
    if(row < 0 || row >= items.size())
        return false;

    ssplib::ItemBase *item = &items[row];
    nodeMgr->requestEditItem(item, EditingModes::StationTrackEditing);

    return true;
}

const ssplib::ItemBase *NodeFinderStationTracksModel::getItemAt(int row)
{
    if(row < 0 || row >= items.size())
        return nullptr;

    return &items.at(row);
}

int NodeFinderStationTracksModel::getItemCount() const
{
    return items.count();
}
