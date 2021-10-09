#include "nodefinderturnoutmodel.h"

#include "editor/manager/nodefindermgr.h"

#include "editor/utils/svgutils.h"

#include <QBrush>

NodeFinderTurnoutModel::NodeFinderTurnoutModel(NodeFinderMgr *mgr, QObject *parent) :
    IObjectModel(parent),
    nodeMgr(mgr)
{

}

QVariant NodeFinderTurnoutModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case StationTrackCol:
            return tr("St. Trk");
        case GateNameCol:
            return tr("Gate");
        case GateTrackCol:
            return tr("Gate Trk");
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int NodeFinderTurnoutModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : items.size();
}

int NodeFinderTurnoutModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : NCols;
}

QVariant NodeFinderTurnoutModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    const TrackConnectionItem& item = items.at(idx.row());

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case StationTrackCol:
            return item.info.stationTrackPos;
        case GateNameCol:
            return item.info.gateLetter;
        case GateTrackCol:
            return item.info.gateTrackPos;
        }
        break;
    }
    case Qt::BackgroundRole:
    {
        if(item.elements.isEmpty())
            return QBrush(Qt::yellow);
        break;
    }
    case Qt::CheckStateRole:
    {
        switch (idx.column())
        {
        case StationTrackCol:
            return item.visible ? Qt::Checked : Qt::Unchecked;
        }
        break;
    }
    }

    return QVariant();
}

bool NodeFinderTurnoutModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (!idx.isValid())
        return false;

    TrackConnectionItem& item = items[idx.row()];

    switch (role)
    {
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case StationTrackCol:
        {
            bool ok = false;
            int trk = value.toInt(&ok);
            if(!ok || trk < 0 || trk > 255) //FIXME: max track?
                return false;

            //Set track
            item.info.stationTrackPos = trk;
            break;
        }
        case GateNameCol:
        {
            QString str = value.toString().simplified();

            if(str.isEmpty())
                return false;

            QChar gateLetter = str.front().toUpper();
            if(gateLetter < 'A' || gateLetter > 'Z')
                return false;

            //Set gate
            item.info.gateLetter = gateLetter;
            break;
        }
        case GateTrackCol:
        {
            bool ok = false;
            int trk = value.toInt(&ok);
            if(!ok || trk < 0 || trk > 255) //FIXME: max track?
                return false;

            //Set track
            item.info.gateTrackPos = trk;
            break;
        }
        }
        break;
    }
    case Qt::CheckStateRole:
    {
        switch (idx.column())
        {
        case StationTrackCol:
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

Qt::ItemFlags NodeFinderTurnoutModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f;
    if(!idx.isValid())
        return f;

    f.setFlag(Qt::ItemIsEnabled);
    f.setFlag(Qt::ItemIsSelectable);
    f.setFlag(Qt::ItemIsEditable);
    if(idx.column() == StationTrackCol)
        f.setFlag(Qt::ItemIsUserCheckable);
    return f;
}

void NodeFinderTurnoutModel::setItems(const QVector<TrackConnectionItem> &vec)
{
    beginResetModel();
    items = vec;
    endResetModel();
}

void NodeFinderTurnoutModel::clear()
{
    beginResetModel();
    items.clear();
    items.squeeze();
    endResetModel();
}

bool NodeFinderTurnoutModel::addElementToItem(ElementPath &p, ItemBase *item)
{
    if(item < items.data() || item >= items.data() + items.size() || item->elements.contains(p))
        return false; //Not a label item

    TrackConnectionItem *ptr = static_cast<TrackConnectionItem *>(item);
    int row = ptr - items.data(); //Pointer aritmetics

    //Rebuild attribute
    QVector<TrackConnectionInfo> infoVec;
    utils::parseTrackConnectionAttribute(p.elem.attribute(svg_attr::TrackConnections), infoVec);
    infoVec.append(ptr->info);
    std::sort(infoVec.begin(), infoVec.end());
    p.elem.setAttribute(svg_attr::TrackConnections, utils::trackConnInfoToString(infoVec));

    item->elements.append(p);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);

    return true;
}

bool NodeFinderTurnoutModel::removeElementFromItem(ItemBase *item, int pos)
{
    if(item < items.data() || item >= items.data() + items.size())
        return false; //Not a label item

    TrackConnectionItem *ptr = static_cast<TrackConnectionItem *>(item);
    int row = ptr - items.data(); //Pointer aritmetics

    ElementPath p = ptr->elements.takeAt(pos);

    //Rebuild attribute
    QVector<TrackConnectionInfo> infoVec;
    utils::parseTrackConnectionAttribute(p.elem.attribute(svg_attr::TrackConnections), infoVec);
    infoVec.removeAll(ptr->info);
    p.elem.setAttribute(svg_attr::TrackConnections, utils::trackConnInfoToString(infoVec));

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);

    return true;
}

bool NodeFinderTurnoutModel::addItem()
{
    nodeMgr->clearCurrentItem();

    TrackConnectionItem item;
    item.info.stationTrackPos = 0;
    item.info.gateLetter = '-';
    item.info.gateTrackPos = 0;
    item.visible = false;

    beginInsertRows(QModelIndex(), items.size(), items.size());
    items.append(item);
    endInsertRows();

    return true;
}

bool NodeFinderTurnoutModel::removeItem(int row)
{
    if(row < 0 || row >= items.size())
        return false;

    nodeMgr->clearCurrentItem();

    ItemBase& item = items[row];

    for(ElementPath& p : item.elements)
    {
        //Rebuild attribute
        QVector<TrackConnectionInfo> infoVec;
        utils::parseTrackConnectionAttribute(p.elem.attribute(svg_attr::TrackConnections), infoVec);
        infoVec.removeAll(items.at(row).info);
        p.elem.setAttribute(svg_attr::TrackConnections, utils::trackConnInfoToString(infoVec));
    }

    beginRemoveRows(QModelIndex(), row, row);
    items.removeAt(row);
    endRemoveRows();

    return true;
}

bool NodeFinderTurnoutModel::editItem(int row)
{
    if(row < 0 || row >= items.size())
        return false;

    ItemBase *item = &items[row];
    nodeMgr->requestEditItem(item, EditingModes::TrackPathEditing);

    return true;
}

const ItemBase* NodeFinderTurnoutModel::getItemAt(int row)
{
    if(row < 0 || row >= items.size())
        return nullptr;

    return &items.at(row);
}

int NodeFinderTurnoutModel::getItemCount() const
{
    return items.count();
}
