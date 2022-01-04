#include "nodefinderturnoutmodel.h"

#include "manager/nodefindermgr.h"

#include <ssplib/utils/svg_path_utils.h>
#include <ssplib/utils/svg_constants.h>
#include <ssplib/stationplan.h>

#include <QBrush>

NodeFinderTurnoutModel::NodeFinderTurnoutModel(NodeFinderMgr *mgr, ssplib::StationPlan *plan, QObject *parent) :
    IObjectModel(parent),
    nodeMgr(mgr),
    m_plan(plan)
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
        case StationTrackSideCol:
            return tr("Side");
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
    return parent.isValid() ? 0 : m_plan->trackConnections.size();
}

int NodeFinderTurnoutModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : NCols;
}

QVariant NodeFinderTurnoutModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    const ssplib::TrackConnectionItem& item = m_plan->trackConnections.at(idx.row());

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (idx.column())
        {
        case StationTrackCol:
            return item.info.stationTrackPos;
        case StationTrackSideCol:
            return getTrackSideName(item.info.trackSide);
        case GateNameCol:
            return item.info.gateLetter;
        case GateTrackCol:
            return item.info.gateTrackPos;
        }
        break;
    }
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case StationTrackCol:
            return item.info.stationTrackPos;
        case StationTrackSideCol:
            return int(item.info.trackSide);
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

    ssplib::TrackConnectionItem& item = m_plan->trackConnections[idx.row()];

    const ssplib::TrackConnectionInfo oldInfo = item.info;

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

            if(item.info.stationTrackPos == trk)
                return false;

            //Set station track
            item.info.stationTrackPos = trk;
            break;
        }
        case StationTrackSideCol:
        {
            bool ok = false;
            int trk = value.toInt(&ok);
            if(!ok || trk < 0 || trk >= int(ssplib::Side::NSides))
                return false;

            const ssplib::Side trkSide = ssplib::Side(trk);
            if(item.info.trackSide == trkSide)
                return false;

            //Set station track
            item.info.trackSide = trkSide;
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

            if(item.info.gateLetter == gateLetter)
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

            if(item.info.gateTrackPos == trk)
                return false;

            //Set gate track
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

    if(!oldInfo.matchNames(item.info))
    {
        //Rebuild element attributes
        for(ssplib::ElementPath &p : item.elements)
        {
            //Rebuild attribute
            QVector<ssplib::TrackConnectionInfo> infoVec;
            ssplib::utils::parseTrackConnectionAttribute(p.elem.attribute(ssplib::svg_attr::TrackConnections), infoVec);
            ssplib::TrackConnectionInfo::removeAllNames(infoVec, oldInfo); //Remove old
            infoVec.append(item.info); //Add new
            std::sort(infoVec.begin(), infoVec.end());
            p.elem.setAttribute(ssplib::svg_attr::TrackConnections, ssplib::utils::trackConnInfoToString(infoVec));
        }
    }

    std::sort(m_plan->trackConnections.begin(), m_plan->trackConnections.end());

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

bool NodeFinderTurnoutModel::addElementToItem(ssplib::ElementPath &p, ssplib::ItemBase *item)
{
    if(item < m_plan->trackConnections.data() || item >= m_plan->trackConnections.data() + m_plan->trackConnections.size() || item->elements.contains(p))
        return false; //Not a label item

    ssplib::TrackConnectionItem *ptr = static_cast<ssplib::TrackConnectionItem *>(item);
    int row = ptr - m_plan->trackConnections.data(); //Pointer aritmetics

    //Rebuild attribute
    QVector<ssplib::TrackConnectionInfo> infoVec;
    ssplib::utils::parseTrackConnectionAttribute(p.elem.attribute(ssplib::svg_attr::TrackConnections), infoVec);
    infoVec.append(ptr->info);
    std::sort(infoVec.begin(), infoVec.end());
    p.elem.setAttribute(ssplib::svg_attr::TrackConnections, ssplib::utils::trackConnInfoToString(infoVec));

    item->elements.append(p);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);

    return true;
}

bool NodeFinderTurnoutModel::removeElementFromItem(ssplib::ItemBase *item, int pos)
{
    if(item < m_plan->trackConnections.data() || item >= m_plan->trackConnections.data() + m_plan->trackConnections.size())
        return false; //Not a label item

    ssplib::TrackConnectionItem *ptr = static_cast<ssplib::TrackConnectionItem *>(item);
    int row = ptr - m_plan->trackConnections.data(); //Pointer aritmetics

    ssplib::ElementPath p = ptr->elements.takeAt(pos);

    //Rebuild attribute
    QVector<ssplib::TrackConnectionInfo> infoVec;
    ssplib::utils::parseTrackConnectionAttribute(p.elem.attribute(ssplib::svg_attr::TrackConnections), infoVec);
    ssplib::TrackConnectionInfo::removeAllNames(infoVec, ptr->info);
    p.elem.setAttribute(ssplib::svg_attr::TrackConnections, ssplib::utils::trackConnInfoToString(infoVec));

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);

    return true;
}

bool NodeFinderTurnoutModel::addItem()
{
    nodeMgr->clearCurrentItem();

    ssplib::TrackConnectionItem item;
    item.info.stationTrackPos = 0;
    item.info.trackSide = ssplib::Side::West;
    item.info.gateLetter = '-';
    item.info.gateTrackPos = 0;
    item.visible = false;

    beginInsertRows(QModelIndex(), m_plan->trackConnections.size(), m_plan->trackConnections.size());
    m_plan->trackConnections.append(item);
    endInsertRows();

    return true;
}

bool NodeFinderTurnoutModel::removeItem(int row)
{
    if(row < 0 || row >= m_plan->trackConnections.size())
        return false;

    nodeMgr->clearCurrentItem();

    ssplib::ItemBase& item = m_plan->trackConnections[row];

    for(ssplib::ElementPath& p : item.elements)
    {
        //Rebuild attribute
        QVector<ssplib::TrackConnectionInfo> infoVec;
        ssplib::utils::parseTrackConnectionAttribute(p.elem.attribute(ssplib::svg_attr::TrackConnections), infoVec);
        ssplib::TrackConnectionInfo::removeAllNames(infoVec, m_plan->trackConnections.at(row).info);
        p.elem.setAttribute(ssplib::svg_attr::TrackConnections, ssplib::utils::trackConnInfoToString(infoVec));
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_plan->trackConnections.removeAt(row);
    endRemoveRows();

    return true;
}

bool NodeFinderTurnoutModel::editItem(int row)
{
    if(row < 0 || row >= m_plan->trackConnections.size())
        return false;

    ssplib::ItemBase *item = &m_plan->trackConnections[row];
    nodeMgr->requestEditItem(item, EditingModes::TrackPathEditing);

    return true;
}
