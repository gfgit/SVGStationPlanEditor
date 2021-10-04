#include "elementsplitterhelper.h"

#include "utils/svgutils.h"
#include "utils/pathutils.h"

#include "nodefindermgr.h"
#include "nodefindersvgconverter.h"

ElementSplitterHelper::ElementSplitterHelper(NodeFinderMgr *mgr, QDomElement e) :
    nodeMgr(mgr),
    origElem(e)
{

}

bool ElementSplitterHelper::splitAt(const QPointF &pos)
{
    QPainterPath path;
    QPainterPath dest;
    QPainterPath rest;

    if(!utils::convertElementToPath(origElem, path))
        return false;

    if(!cutPathAtPoint(pos, path, dest, rest))
        return false;

    QString destVal;
    if(!utils::convertPathToSVG(dest, destVal))
        return false;

    QString restVal;
    if(!utils::convertPathToSVG(rest, restVal))
        return false;

    if(origElem.tagName() != svg_tag::PathTag)
    {
        //Remove from original class
        bool isFakeId = false;
        nodeMgr->getConverter()->removeElement(origElem, &isFakeId);

        if(isFakeId)
        {
            //Clear ID to trigger generation of new one.
            nodeMgr->getConverter()->renameElement(origElem, QString());
        }

        //Convert to path
        origElem.setTagName(svg_tag::PathTag);

        QStringList attrs{
            "points",
            "x", "x1", "x2",
            "y", "y1", "y2",
            "height", "width"
        };

        for(const QString& attr : attrs)
            origElem.removeAttribute(attr);

        //Add to path class
        nodeMgr->getConverter()->storeElement(origElem);
    }

    //Copy
    QDomElement newElem = origElem.cloneNode().toElement();
    origElem.parentNode().insertAfter(newElem, origElem);

    origElem.setAttribute("d", destVal);
    newElem.setAttribute("d", restVal);

    //Generate new ID
    int counter = 0;
    const QString base = origElem.attribute(svg_attr::ID);
    QString id = nodeMgr->getConverter()->getFreeId_internal(base, counter);

    newElem.setAttribute(svg_attr::ID, id);

    //Store new element
    nodeMgr->getConverter()->storeElement(newElem);

    return true;
}
