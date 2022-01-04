#ifndef IOBJECTMODEL_H
#define IOBJECTMODEL_H

#include <QAbstractTableModel>

#include <ssplib/itemtypes.h>

namespace ssplib {
struct ItemBase;
struct ElementPath;
} // namespace ssplib

class IObjectModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit IObjectModel(QObject *parent = nullptr);

    void refreshModel();

    virtual bool addItem();
    virtual bool editItem(int row);
    virtual bool removeItem(int row);

    virtual bool addElementToItem(ssplib::ElementPath &p, ssplib::ItemBase *item);
    virtual bool removeElementFromItem(ssplib::ItemBase *item, int pos);

    //Common translations
    static inline QString getTrackSideName(ssplib::Side s)
    {
        switch (s)
        {
        case ssplib::Side::East:
            return tr("East");
        case ssplib::Side::West:
            return tr("West");
        default:
            break;
        }
        return QString();
    }

    static constexpr const char *errMsgCannotEditWithXML =
        QT_TRANSLATE_NOOP("IObjectModel",
                          "Cannot edit item loaded from XML.\n"
                          "Unload XML to edit this item.");
    static constexpr const char *errMsgCannotRemWithXML =
        QT_TRANSLATE_NOOP("IObjectModel",
                          "Cannot remove item loaded from XML.\n"
                          "Unload XML to remove this item.");

signals:
    void itemRemoved(int row);
    void errorOccurred(const QString& msg);
};

#endif // IOBJECTMODEL_H
