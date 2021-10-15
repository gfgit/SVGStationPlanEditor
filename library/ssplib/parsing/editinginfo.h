#ifndef SSPLIB_EDITINGINFO_H
#define SSPLIB_EDITINGINFO_H

#ifdef SSPLIB_ENABLE_EDITING

#include <QVector>
#include <QDomElement>

#include <QMap>

namespace ssplib {

typedef QMap<QString, QDomElement> ElementMap;

class ItemEditBase
{
public:
    QVector<QDomElement> elements;
};

class EditingInfo
{
public:
    EditingInfo();

    void storeElement(QDomElement &e);

    void resetIdGenerator();
    QString getFreeId_internal(const QString &base, int &counter);

public:
    QVector<ItemEditBase> labels;
    QVector<ItemEditBase> platforms;
    QVector<ItemEditBase> trackConnections;

public:
    ElementMap namedElements;

private:
    int generatedIdSerial;
    QString generatedIdBase;
};

} // namespace ssplib

#endif // SSPLIB_ENABLE_EDITING

#endif // SSPLIB_EDITINGINFO_H
