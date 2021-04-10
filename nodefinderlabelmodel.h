#ifndef NODEFINDERLABELMODEL_H
#define NODEFINDERLABELMODEL_H

#include <QAbstractTableModel>

#include <QVector>

#include <QDomElement>
#include <QRectF>

class NodeFinderMgr;

class NodeFinderLabelModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    enum Columns
    {
        LabelNameCol = 0,
        NCols
    };

    typedef struct LabelItem
    {
        QChar gateLetter;
        QDomElement elem;
        QRectF rect;
        bool visible;
    } LabelItem;

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
    void clear();

private:
    friend class NodeFinderSVGWidget;

    NodeFinderMgr *nodeMgr;
    QVector<LabelItem> items;
};

#endif // NODEFINDERLABELMODEL_H
