#ifndef ABSTRACTLAYOUTITEM_H
#define ABSTRACTLAYOUTITEM_H

#include <QVector>
#include <QVariantMap>
#include <QString>

class QXmlStreamWriter;
class QGraphicsScene;

class AbstractLayoutItem
{
public:
    AbstractLayoutItem();
    virtual ~AbstractLayoutItem();


    struct PropertyDescr
    {
        QString name;
        QString description;
        QVariant::Type type = QVariant::Invalid;
        QStringList enumNames = {};
        double min = 0;
        double max = 0;
    };

    virtual QString itemType() = 0;

    virtual QVector<PropertyDescr> getPropertyDescriptors() = 0;

    virtual QVariantMap getProperties() = 0;

    virtual bool setProperties(const QVariantMap& map) = 0;

    virtual bool addToScene(QGraphicsScene *scene) = 0;

    virtual void writeSVGElements(QXmlStreamWriter& xml) = 0;
};

#endif // ABSTRACTLAYOUTITEM_H
