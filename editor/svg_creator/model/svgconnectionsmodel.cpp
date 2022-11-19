#include "svgconnectionsmodel.h"

#include "svg_creator/svgcreatormanager.h"

#include <QGraphicsLineItem>
#include <QPen>

SvgConnectionsModel::SvgConnectionsModel(QObject *parent) :
    QAbstractTableModel(parent)
{

}

QVariant SvgConnectionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case GateLetterCol:
            return tr("Gate");
        case GateTrackCol:
            return tr("Tr N.");
        case PlatformCol:
            return tr("Platform");
        case PlatfSideCol:
            return tr("Side");
        default:
            break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int SvgConnectionsModel::rowCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : m_data.size();
}

int SvgConnectionsModel::columnCount(const QModelIndex &p) const
{
    return p.isValid() ? 0 : NCols;
}

QVariant SvgConnectionsModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= m_data.size())
        return QVariant();

    const GateConnectionData &item = m_data.at(idx.row());

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (idx.column())
        {
        case GateLetterCol:
            return item.gateLetter;
        case GateTrackCol:
            return item.gateTrackNum;
        case PlatformCol:
            return item.platfName;
        case PlatfSideCol:
            return item.westSide ? tr("West") : tr("East");
        default:
            break;
        }
        break;
    }
    case Qt::CheckStateRole:
    {
        switch (idx.column())
        {
        case GateLetterCol:
            return item.visible ? Qt::Checked : Qt::Unchecked;
        }
        break;
    }
    default:
        break;
    }

    return QVariant();
}

bool SvgConnectionsModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (!idx.isValid() || idx.row() >= m_data.size())
        return false;

    GateConnectionData &item = m_data[idx.row()];

    bool oldValue = item.visible;

    switch (role)
    {
    case Qt::CheckStateRole:
    {
        switch (idx.column())
        {
        case GateLetterCol:
        {
            Qt::CheckState cs = value.value<Qt::CheckState>();
            item.visible = cs == Qt::Checked;
        }
        }
        break;
    }
    }

    if(item.visible == oldValue)
        return false;


    if(item.visible)
    {
        //Highlight items
        QPen pen(Qt::red, 7);

        for(auto line : qAsConst(item.items))
        {
            line->setData(GraphicsOldPenKey, line->pen());
            line->setPen(pen);
        }
    }
    else
    {
        //Reset to original color
        for(auto line : qAsConst(item.items))
        {
            QPen pen = line->data(GraphicsOldPenKey).value<QPen>();
            line->setData(GraphicsOldPenKey, QVariant());
            line->setPen(pen);
        }
    }

    emit dataChanged(idx, idx);
    return true;
}

Qt::ItemFlags SvgConnectionsModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f;
    if(!idx.isValid())
        return f;

    f.setFlag(Qt::ItemIsEnabled);
    f.setFlag(Qt::ItemIsSelectable);

    if(idx.column() == GateLetterCol)
        f.setFlag(Qt::ItemIsUserCheckable);

    return f;
}

void SvgConnectionsModel::setConnections(const QVector<GateConnectionData> &items)
{
    beginResetModel();
    m_data = items;
    endResetModel();
}
