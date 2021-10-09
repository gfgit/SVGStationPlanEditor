#ifndef NODEFINDERTURNOUTMODEL_H
#define NODEFINDERTURNOUTMODEL_H

#include "iobjectmodel.h"

#include <QVector>

#include "editor/utils/nodefindertypes.h"

class NodeFinderMgr;

class NodeFinderTurnoutModel : public IObjectModel
{
    Q_OBJECT
public:
    enum Columns
    {
        StationTrackCol = 0,
        GateNameCol,
        GateTrackCol,
        NCols
    };

    explicit NodeFinderTurnoutModel(NodeFinderMgr *mgr, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    void setItems(const QVector<TrackConnectionItem> &vec);

    //IObjectModel
    bool addItem() override;
    bool removeItem(int row) override;
    bool editItem(int row) override;

    void clear() override;

    bool addElementToItem(ElementPath &p, ItemBase *item) override;
    bool removeElementFromItem(ItemBase *item, int pos) override;

    const ItemBase *getItemAt(int row) override;

    int getItemCount() const override;

private:
    friend class NodeFinderSVGWidget;
    NodeFinderMgr *nodeMgr;
    QVector<TrackConnectionItem> items;
};

#endif // NODEFINDERTURNOUTMODEL_H
