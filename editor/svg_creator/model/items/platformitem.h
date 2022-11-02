#ifndef PLATFORMITEM_H
#define PLATFORMITEM_H

#include "abstractlayoutitem.h"

class QGraphicsLineItem;

class PlatformItem : public AbstractLayoutItem
{
public:
    PlatformItem();

    QString itemType() override;

    QVector<PropertyDescr> getPropertyDescriptors() override;

    QVariantMap getProperties() override;

    bool setProperties(const QVariantMap& map) override;

    bool addToScene(QGraphicsScene *scene) override;

    void writeSVGElements(QXmlStreamWriter& xml) override;

private:
    QString platfName;
    int platfNum;

    QGraphicsLineItem *platfLineItem = nullptr;
};

#endif // PLATFORMITEM_H
