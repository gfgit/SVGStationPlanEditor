#include "nodefinderlabelmodel.h"

#include "nodefindermgr.h"

NodeFinderLabelModel::NodeFinderLabelModel(NodeFinderMgr *mgr, QObject *parent) :
    QAbstractTableModel(parent),
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
    case Qt::DisplayRole:
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

void NodeFinderLabelModel::setItems(const QVector<NodeFinderLabelModel::LabelItem> &vec)
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
