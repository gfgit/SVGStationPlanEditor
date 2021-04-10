#include "nodefinderlabelmodel.h"

NodeFinderLabelModel::NodeFinderLabelModel(QObject *parent)
    : QAbstractTableModel(parent)
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
    if (!idx.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const LabelItem& item = items.at(idx.row());
    switch (idx.column())
    {
    case LabelNameCol:
        return item.gateLetter;
    }

    return QVariant();
}

void NodeFinderLabelModel::setItems(const QVector<NodeFinderLabelModel::LabelItem> &vec)
{
    beginResetModel();
    items = vec;
    endResetModel();
}
