#include "elementsplitterhelper.h"

#include "ssplib/utils/svg_path_utils.h"
#include "ssplib/utils/svg_constants.h"
#include "utils/pathutils.h"

#include "nodefindermgr.h"
#include "nodefindersvgconverter.h"

ElementSplitterHelper::ElementSplitterHelper(NodeFinderMgr *mgr, QDomElement e, double threshold) :
    nodeMgr(mgr),
    origElem(e),
    m_threshold(threshold)
{

}

bool ElementSplitterHelper::splitAt(const QPointF &pos)
{
    QPainterPath path;
    QPainterPath dest;
    QPainterPath rest;

    if(!ssplib::utils::convertElementToPath(origElem, path))
        return false;

    if(!cutPathAtPoint(pos, m_threshold, path, dest, rest))
        return false;

    QString destVal;
    if(!ssplib::utils::convertPathToSVG(dest, destVal))
        return false;

    QString restVal;
    if(!ssplib::utils::convertPathToSVG(rest, restVal))
        return false;

    if(origElem.tagName() != ssplib::svg_tags::PathTag)
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
        origElem.setTagName(ssplib::svg_tags::PathTag);

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
    const QString base = origElem.attribute(ssplib::svg_attr::ID);
    QString id = nodeMgr->getConverter()->getFreeId_internal(base, counter);

    newElem.setAttribute(ssplib::svg_attr::ID, id);

    //Store new element
    nodeMgr->getConverter()->storeElement(newElem);

    return true;
}
