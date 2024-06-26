#ifndef NODEFINDERSTATIONTRACKSMODEL_H
#define NODEFINDERSTATIONTRACKSMODEL_H

#include "iobjectmodel.h"

#include <QList>

#include <ssplib/itemtypes.h>

namespace ssplib {
class StationPlan;
} // namespace ssplib

class NodeFinderMgr;

class NodeFinderStationTracksModel : public IObjectModel
{
    Q_OBJECT

public:
    enum Columns
    {
        TrackNameCol = 0,
        TrackPosCol,
        SpecialMixedColumn,
        NCols
    };

    explicit NodeFinderStationTracksModel(ssplib::StationPlan *plan, ssplib::StationPlan *xml,
                                          NodeFinderMgr *mgr, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    void clearElement(ssplib::ElementPath &elemPath);

    //IObjectModel
    bool addItem() override;
    bool removeItem(int row) override;
    bool editItem(int row) override;

    bool addElementToItem(ssplib::ElementPath &p, ssplib::ItemBase *item) override;
    bool removeElementFromItem(ssplib::ItemBase *item, int pos) override;

    bool itemIsInXML(const ssplib::TrackItem &item) const;

signals:
    void tracksChanged();

private:
    friend class NodeFinderSVGWidget;
    NodeFinderMgr *nodeMgr;
    ssplib::StationPlan *m_plan;
    ssplib::StationPlan *xmlPlan;
};

#endif // NODEFINDERSTATIONTRACKSMODEL_H
