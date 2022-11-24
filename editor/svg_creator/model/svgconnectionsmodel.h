#ifndef SVGCONNECTIONSMODEL_H
#define SVGCONNECTIONSMODEL_H

#include <QAbstractTableModel>

#include <QVector>

class QGraphicsLineItem;

struct GateConnectionData
{
    QString platfName;
    QChar gateLetter;
    int gateTrackNum = -1;
    int platfNum = -1;

    //This is the sum of abs(angle) for every node
    double totalRotations = 0;

    bool westSide = false;
    bool visible = false;

    QVector<QGraphicsLineItem *> items;
};

class SvgConnectionsModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    enum Column
    {
        GateLetterCol = 0,
        GateTrackCol,
        PlatformCol,
        PlatfSideCol,
        TotalAngleCol,
        NCols
    };

    explicit SvgConnectionsModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &p = QModelIndex()) const override;
    int columnCount(const QModelIndex &p = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    void setConnections(const QVector<GateConnectionData>& items);

private:
    QVector<GateConnectionData> m_data;
};

#endif // SVGCONNECTIONSMODEL_H
