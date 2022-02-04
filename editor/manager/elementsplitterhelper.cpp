#include "elementsplitterhelper.h"

#include <ssplib/utils/svg_path_utils.h>
#include <ssplib/utils/svg_constants.h>
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

    if(!convertToPath(origElem, dest, nodeMgr))
        return false;

    QString restVal;
    if(!ssplib::utils::convertPathToSVG(rest, restVal))
        return false;

    convertToPath(origElem, dest, nodeMgr);

    //Copy
    QDomElement newElem = origElem.cloneNode().toElement();
    origElem.parentNode().insertAfter(newElem, origElem);

    newElem.setAttribute("d", restVal);

    //Generate new ID
    int counter = 0;
    const QString base = origElem.attribute(ssplib::svg_attr::ID);
    QString id = nodeMgr->getConverter()->m_info.getFreeId_internal(base, counter);

    newElem.setAttribute(ssplib::svg_attr::ID, id);

    //Store new element
    nodeMgr->getConverter()->storeElement(newElem);

    return true;
}

bool ElementSplitterHelper::convertToPath(QDomElement &e, const QPainterPath &path, NodeFinderMgr *mgr)
{
    //Morph into path element
    if(e.tagName() != ssplib::svg_tags::PathTag)
    {
        //Remove from original class
        bool isFakeId = false;
        mgr->getConverter()->removeElement(e, &isFakeId);

        if(isFakeId)
        {
            //Clear ID to trigger generation of new one.
            mgr->getConverter()->renameElement(e, QString());
        }

        //Convert to path
        e.setTagName(ssplib::svg_tags::PathTag);

        QStringList attrs{
            "points",
            "x", "x1", "x2",
            "y", "y1", "y2",
            "height", "width"
        };

        for(const QString& attr : attrs)
            e.removeAttribute(attr);

        //Add to path class
        mgr->getConverter()->storeElement(e);
    }

    //Build and set path
    QString destVal;
    if(!ssplib::utils::convertPathToSVG(path, destVal))
        return false;

    e.setAttribute("d", destVal);
    return true;
}
