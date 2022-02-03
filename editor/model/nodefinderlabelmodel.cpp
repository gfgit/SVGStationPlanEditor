#include "nodefinderlabelmodel.h"

#include "manager/nodefindermgr.h"

#include <ssplib/utils/svg_constants.h>

#include <ssplib/stationplan.h>

#include <QBrush>

NodeFinderLabelModel::NodeFinderLabelModel(ssplib::StationPlan *plan, ssplib::StationPlan *xml,
                                           NodeFinderMgr *mgr, QObject *parent) :
    IObjectModel(parent),
    nodeMgr(mgr),
    m_plan(plan),
    xmlPlan(xml)
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
        case TrackCountCol:
            return tr("Trk Count");
        case GateSideCol:
            return tr("Side");
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int NodeFinderLabelModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_plan->labels.size();
}

int NodeFinderLabelModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : NCols;
}

QVariant NodeFinderLabelModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    const ssplib::LabelItem& item = m_plan->labels.at(idx.row());

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (idx.column())
        {
        case LabelNameCol:
            return item.gateLetter;
        case TrackCountCol:
            return item.gateOutTrkCount;
        case GateSideCol:
            return getTrackSideName(item.gateSide);
        }
        break;
    }
    case Qt::EditRole:
    {
        switch (idx.column())
        {
        case LabelNameCol:
            return item.gateLetter;
        case TrackCountCol:
            return item.gateOutTrkCount;
        case GateSideCol:
            return int(item.gateSide);
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


    ssplib::LabelItem& item = m_plan->labels[idx.row()];
    const QChar oldGateLetter = item.gateLetter;

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
        case LabelNameCol:
        {
            const QString name = value.toString().simplified();
            if(name.isEmpty() || name.front() < 'A' || name.front() > 'Z')
            {
                emit errorOccurred(tr("Gate name must be a letter"));
                return false;
            }

            const QChar letter = name.front();

            if(item.gateLetter == letter)
                return false;

            for(const ssplib::LabelItem& gate : qAsConst(m_plan->labels))
            {
                if(gate.gateLetter == letter)
                {
                    emit errorOccurred(tr("Gate <b>%1</b> already exists").arg(letter));
                    return false; //Name already exists
                }
            }

            //Set name
            item.gateLetter = letter;
            break;
        }
        case TrackCountCol:
        {
            bool ok = false;
            int trkCount = value.toInt(&ok);
            if(!ok || trkCount < 1 || trkCount > 255) //TODO: max track?
                return false;

            if(item.gateOutTrkCount == trkCount)
                return false;

            //Set track count
            item.gateOutTrkCount = trkCount;
            break;
        }
        case GateSideCol:
        {
            bool ok = false;
            int side = value.toInt(&ok);
            if(!ok || side < 0 || side >= int(ssplib::Side::NSides))
                return false;

            const ssplib::Side gateSide = ssplib::Side(side);
            if(item.gateSide == gateSide)
                return false;

            //Set station track
            item.gateSide = gateSide;
            break;
        }
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

    if(oldGateLetter != item.gateLetter)
    {
        //Rebuild element attributes
        for(ssplib::ElementPath &p : item.elements)
        {
            //Rebuild attribute
            p.elem.setAttribute(ssplib::svg_attr::LabelName, item.gateLetter);
        }
    }

    std::sort(m_plan->labels.begin(), m_plan->labels.end());

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

bool NodeFinderLabelModel::addElementToItem(ssplib::ElementPath &p, ssplib::ItemBase *item)
{
    if(item < m_plan->labels.data() || item >= m_plan->labels.data() + m_plan->labels.size() || item->elements.contains(p))
        return false; //Not a label item

    ssplib::LabelItem *ptr = static_cast<ssplib::LabelItem *>(item);
    int row = ptr - m_plan->labels.data(); //Pointer aritmetics

    p.elem.setAttribute(ssplib::svg_attr::LabelName, ptr->gateLetter);
    item->elements.append(p);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);

    return true;
}

bool NodeFinderLabelModel::removeElementFromItem(ssplib::ItemBase *item, int pos)
{
    if(item < m_plan->labels.data() || item >= m_plan->labels.data() + m_plan->labels.size())
        return false; //Not a label item

    ssplib::LabelItem *ptr = static_cast<ssplib::LabelItem *>(item);
    int row = ptr - m_plan->labels.data(); //Pointer aritmetics

    ptr->elements[pos].elem.removeAttribute(ssplib::svg_attr::LabelName);
    ptr->elements.removeAt(pos);

    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx);

    return true;
}

bool NodeFinderLabelModel::itemIsInXML(const ssplib::LabelItem &item) const
{
    for(const ssplib::LabelItem& gate : qAsConst(xmlPlan->labels))
    {
        if(gate.gateLetter == item.gateLetter)
            return true;
    }
    return false;
}

bool NodeFinderLabelModel::addItem()
{
    nodeMgr->clearCurrentItem();

    ssplib::LabelItem item;
    item.gateLetter = '-';
    item.visible = false;

    beginInsertRows(QModelIndex(), m_plan->labels.size(), m_plan->labels.size());
    m_plan->labels.append(item);
    endInsertRows();

    return true;
}

bool NodeFinderLabelModel::removeItem(int row)
{
    if(row < 0 || row >= m_plan->labels.size())
        return false;

    nodeMgr->clearCurrentItem();

    ssplib::LabelItem& item = m_plan->labels[row];
    if(itemIsInXML(item))
    {
        emit errorOccurred(IObjectModel::tr(errMsgCannotRemWithXML));
        return false;
    }

    for(ssplib::ElementPath& elemPath : item.elements)
    {
        elemPath.elem.removeAttribute(ssplib::svg_attr::LabelName);
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_plan->labels.removeAt(row);
    endRemoveRows();

    return true;
}

bool NodeFinderLabelModel::editItem(int row)
{
    if(row < 0 || row >= m_plan->labels.size())
        return false;

    ssplib::ItemBase *item = &m_plan->labels[row];
    nodeMgr->requestEditItem(item, EditingModes::LabelEditing);

    return true;
}
