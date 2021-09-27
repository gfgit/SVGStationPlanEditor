#ifndef NODEFINDERLABELMODEL_H
#define NODEFINDERLABELMODEL_H

#include "iobjectmodel.h"

#include <QVector>

#include "utils/nodefinderutils.h"

class NodeFinderMgr;

class NodeFinderLabelModel : public IObjectModel
{
    Q_OBJECT

public:

    enum Columns
    {
        LabelNameCol = 0,
        NCols
    };

    explicit NodeFinderLabelModel(NodeFinderMgr *mgr, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    void setItems(const QVector<LabelItem>& vec);

    //IObjectModel
    void clear() override;

    bool addElementToItem(ElementPath &p, ItemBase *item) override;

private:
    friend class NodeFinderSVGWidget;

    NodeFinderMgr *nodeMgr;
    QVector<LabelItem> items;
};

#endif // NODEFINDERLABELMODEL_H
