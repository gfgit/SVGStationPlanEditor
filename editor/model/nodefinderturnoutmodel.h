#ifndef NODEFINDERTURNOUTMODEL_H
#define NODEFINDERTURNOUTMODEL_H

#include "iobjectmodel.h"

#include <QVector>

#include <ssplib/itemtypes.h>

namespace ssplib {
class StationPlan;
} // namespace ssplib

class NodeFinderMgr;

class NodeFinderTurnoutModel : public IObjectModel
{
    Q_OBJECT
public:
    enum Columns
    {
        StationTrackCol = 0,
        StationTrackSideCol,
        GateNameCol,
        GateTrackCol,
        NCols
    };

    explicit NodeFinderTurnoutModel(ssplib::StationPlan *plan, ssplib::StationPlan *xml,
                                    NodeFinderMgr *mgr, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    //IObjectModel
    bool addItem() override;
    bool removeItem(int row) override;
    bool editItem(int row) override;

    bool addElementToItem(ssplib::ElementPath &p, ssplib::ItemBase *item) override;
    bool removeElementFromItem(ssplib::ItemBase *item, int pos) override;

    bool itemIsInXML(const ssplib::TrackConnectionItem &item) const;

public slots:
    void refreshData();

private:
    friend class NodeFinderSVGWidget;
    NodeFinderMgr *nodeMgr;
    ssplib::StationPlan *m_plan;
    ssplib::StationPlan *xmlPlan;
};

#endif // NODEFINDERTURNOUTMODEL_H
