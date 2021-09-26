#ifndef NODEFINDERSTATIONTRACKSMODEL_H
#define NODEFINDERSTATIONTRACKSMODEL_H

#include <QAbstractItemModel>

#include <QVector>

#include "utils/nodefinderutils.h"

class NodeFinderMgr;

class NodeFinderStationTracksModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        TrackNameCol = 0,
        NCols
    };

    explicit NodeFinderStationTracksModel(NodeFinderMgr *mgr, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    void setItems(const QVector<TrackItem>& vec);

    void clear();

    int getTrackPos(const ItemBase *ptr) const;

    void clearElement(ElementPath &elemPath);

    void addItem();
    void removeItem(int row);
    void editItemAt(int row);

private:
    friend class NodeFinderSVGWidget;
    NodeFinderMgr *nodeMgr;
    QVector<TrackItem> items;
};

#endif // NODEFINDERSTATIONTRACKSMODEL_H
