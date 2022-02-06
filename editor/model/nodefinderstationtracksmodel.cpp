#include "nodefinderstationtracksmodel.h"

#include "manager/nodefindermgr.h"

#include <ssplib/utils/svg_constants.h>
#include <ssplib/stationplan.h>

#include <QBrush>

NodeFinderStationTracksModel::NodeFinderStationTracksModel(ssplib::StationPlan *plan, ssplib::StationPlan *xml,
                                                           NodeFinderMgr *mgr, QObject *parent) :
    IObjectModel(parent),
    nodeMgr(mgr),
    m_plan(plan),
    xmlPlan(xml)
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
        case TrackPosCol:
            return tr("Pos");
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int NodeFinderStationTracksModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_plan->platforms.size();
}

int NodeFinderStationTracksModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : NCols;
}

QVariant NodeFinderStationTracksModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    const ssplib::TrackItem& item = m_plan->platforms.at(idx.row());

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (idx.column())
        {
        case TrackNameCol:
            return item.trackName;
        case TrackPosCol:
            return item.trackPos;
        case SpecialMixedColumn: //Mixed colummn for completion
            return item.trackName.isEmpty() ? QString("#%1").arg(item.trackPos) : item.trackName;
        }
        break;
    }
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case TrackNameCol:
            return item.trackName;
        case TrackPosCol:
            return item.trackPos;
        case SpecialMixedColumn: //Mixed colummn for completion
            return item.trackPos;
        }
        break;
    }
    case Qt::BackgroundRole:
    {
        if(!itemIsInXML(item))
            return QBrush(Qt::red);
        if(item.elements.isEmpty())
            return QBrush(Qt::yellow);
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

    ssplib::TrackItem& item = m_plan->platforms[idx.row()];
    const int oldTrackPos = item.trackPos;

    if(itemIsInXML(item) && role != Qt::CheckStateRole)
    {
        //For items in XML allow only to set visible/non visible
        emit errorOccurred(IObjectModel::tr(errMsgCannotEditWithXML));
        return false;
    }

    switch (role)
    {
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case TrackNameCol:
        {
            QString name = value.toString();
            if(name.isEmpty())
                return false;

            if(item.trackName == name)
                return false;

            for(const ssplib::TrackItem& track : qAsConst(m_plan->platforms))
            {
                if(track.trackName == name)
                {
                    emit errorOccurred(tr("Track Name <b>%1</b> is already used.")
                                           .arg(track.trackName));
                    return false;
                }
            }

            //Set name
            item.trackName = name;
            break;
        }
        case TrackPosCol:
        {
            bool ok = false;
            int trk = value.toInt(&ok);
            if(!ok || trk < 0)
                return false;

            if(item.trackPos == trk)
                return false;

            for(const ssplib::TrackItem& track : qAsConst(m_plan->platforms))
            {
                if(track.trackPos == trk)
                {
                    emit errorOccurred(tr("Track Pos <b>%1</b> is already used by track <b>%2</b>")
                                           .arg(trk).arg(track.trackName));
                    return false;
                }
            }

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

    if(oldTrackPos != item.trackPos)
    {
        //Rebuild element attributes
        const QString trkPosStr = QString::number(item.trackPos);
        for(ssplib::ElementPath &p : item.elements)
        {
            //Rebuild attribute
            p.elem.setAttribute(ssplib::svg_attr::TrackPos, trkPosStr);
        }
    }

    std::sort(m_plan->platforms.begin(), m_plan->platforms.end());

    emit dataChanged(idx, idx);
    emit nodeMgr->repaintSVG();

    emit tracksChanged();

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

bool NodeFinderStationTracksModel::addElementToItem(ssplib::ElementPath &p, ssplib::ItemBase *item)
{
    if(item < m_plan->platforms.data() || item >= m_plan->platforms.data() + m_plan->platforms.size() || item->elements.contains(p))
        return false; //Not a label item

    ssplib::TrackItem *ptr = static_cast<ssplib::TrackItem *>(item);
    int row = ptr - m_plan->platforms.data(); //Pointer aritmetics

    p.elem.setAttribute(ssplib::svg_attr::TrackPos, QString::number(ptr->trackPos));
    item->elements.append(p);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);

    return true;
}

bool NodeFinderStationTracksModel::removeElementFromItem(ssplib::ItemBase *item, int pos)
{
    if(item < m_plan->platforms.data() || item >= m_plan->platforms.data() + m_plan->platforms.size())
        return false; //Not a label item

    ssplib::TrackItem *ptr = static_cast<ssplib::TrackItem *>(item);
    int row = ptr - m_plan->platforms.data(); //Pointer aritmetics

    clearElement(ptr->elements[pos]);
    ptr->elements.removeAt(pos);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);

    return true;
}

bool NodeFinderStationTracksModel::itemIsInXML(const ssplib::TrackItem &item) const
{
    for(const ssplib::TrackItem& track : qAsConst(xmlPlan->platforms))
    {
        if(track.trackPos == item.trackPos)
            return true;
    }
    return false;
}

bool NodeFinderStationTracksModel::addItem()
{
    nodeMgr->clearCurrentItem();

    int maxTrackPos = -1;
    for(const ssplib::TrackItem& track : qAsConst(m_plan->platforms))
    {
        if(track.trackPos > maxTrackPos)
            maxTrackPos = track.trackPos;
    }

    ssplib::TrackItem item;
    item.trackPos = maxTrackPos + 1;
    item.visible = false;

    beginInsertRows(QModelIndex(), m_plan->platforms.size(), m_plan->platforms.size());
    m_plan->platforms.append(item);
    endInsertRows();

    emit tracksChanged();

    return true;
}

void NodeFinderStationTracksModel::clearElement(ssplib::ElementPath &elemPath)
{
    elemPath.elem.removeAttribute(ssplib::svg_attr::TrackPos);
}

bool NodeFinderStationTracksModel::removeItem(int row)
{
    if(row < 0 || row >= m_plan->platforms.size())
        return false;

    nodeMgr->clearCurrentItem();

    ssplib::TrackItem& item = m_plan->platforms[row];
    if(itemIsInXML(item))
    {
        emit errorOccurred(IObjectModel::tr(errMsgCannotRemWithXML));
        return false;
    }

    for(ssplib::ElementPath& elemPath : item.elements)
        clearElement(elemPath);

    beginRemoveRows(QModelIndex(), row, row);
    m_plan->platforms.removeAt(row);
    endRemoveRows();

    emit tracksChanged();

    return true;
}

bool NodeFinderStationTracksModel::editItem(int row)
{
    if(row < 0 || row >= m_plan->platforms.size())
        return false;

    ssplib::ItemBase *item = &m_plan->platforms[row];
    nodeMgr->requestEditItem(item, EditingModes::StationTrackEditing);

    return true;
}
