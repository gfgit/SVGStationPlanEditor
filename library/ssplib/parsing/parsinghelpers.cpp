#include "parsinghelpers.h"


int ssplib::parsing::parseLabel(utils::XmlElement &e, QVector<LabelItem> &labels)
{
    QString labelName = e.attribute(svg_attr::LabelName);
    if(labelName.isEmpty())
        return -1;

    bool ok = true;
    labelName = labelName.simplified();

    if(labelName.isEmpty() || labelName.front() < 'A' || labelName.front() > 'Z')
    {
        ok = false;
    }

    ElementPath elemPath;

    if(ok)
    {
        ok = utils::convertElementToPath(e, elemPath.path);
    }

    if(!ok)
    {
        //Cannot parse attribute or element path, remove it
        e.removeAttribute(svg_attr::LabelName);
        return -1;
    }

    QChar gateLetter = labelName.front();

    int i = 0;
    for(; i < labels.size(); i++)
    {
        if(labels.at(i).gateLetter == gateLetter)
            break; //Label already exists, add the new element
    }
    if(i >= labels.size())
    {
        //Create new label
        LabelItem newItem;
        newItem.gateLetter = gateLetter;
        newItem.visible = false;
        labels.append(newItem);
        i = labels.size() - 1;
    }

    elemPath.strokeWidth = 0;
    if(!utils::parseStrokeWidth(e, elemPath.path.boundingRect(), elemPath.strokeWidth))
        elemPath.strokeWidth = 0;

    //Add element to label
    LabelItem &item = labels[i];
    item.elements.append(elemPath);

    return i;
}

int ssplib::parsing::parsePlatform(utils::XmlElement &e, QVector<TrackItem> &platforms)
{
    QString trackPosStr = e.attribute(svg_attr::TrackPos);
    if(trackPosStr.isEmpty())
        return -1;

    bool ok = false;
    int trackPos = trackPosStr.toInt(&ok);

    ElementPath elemPath;

    if(ok)
    {
        ok = utils::convertElementToPath(e, elemPath.path);
    }

    if(!ok)
    {
        //Cannot parse attribute or element path, remove it
        e.removeAttribute(svg_attr::TrackPos);
        return -1;
    }

    int i = 0;
    for(; i < platforms.size(); i++)
    {
        if(platforms.at(i).trackPos == trackPos)
            break; //Platform exists
    }
    if(i >= platforms.size())
    {
        //Create new platform
        TrackItem newItem;
        //newItem.trackName = ...; //TODO: real name from database
        newItem.trackPos = trackPos;
        newItem.visible = false;
        platforms.append(newItem);
        i = platforms.size() - 1;
    }

    elemPath.strokeWidth = 0;
    if(!utils::parseStrokeWidth(e, elemPath.path.boundingRect(), elemPath.strokeWidth))
        elemPath.strokeWidth = 0;

    //Add element to platform
    TrackItem &item = platforms[i];
    item.elements.append(elemPath);

    return i;
}

bool ssplib::parsing::parseTrackConnection(utils::XmlElement &e,
                                           QVector<TrackConnectionItem> &connections,
                                           QVector<int> *indexes)
{
    QString trackConnStr = e.attribute(svg_attr::TrackConnections);
    if(trackConnStr.isEmpty())
        return false;

    QVector<TrackConnectionInfo> infoVec;
    bool ok = utils::parseTrackConnectionAttribute(trackConnStr, infoVec);

    ElementPath elemPath;

    if(ok)
    {
        ok = utils::convertElementToPath(e, elemPath.path);
    }

    if(!ok)
    {
        //Cannot parse attribute or element path, remove it
        e.removeAttribute(svg_attr::TrackConnections);
        return false;
    }

    elemPath.strokeWidth = 0;
    if(!utils::parseStrokeWidth(e, elemPath.path.boundingRect(), elemPath.strokeWidth))
        elemPath.strokeWidth = 0;

    for(const TrackConnectionInfo& info : qAsConst(infoVec))
    {
        //Find track connection
        int i = 0;
        for(; i < connections.size(); i++)
        {
            if(connections.at(i).info == info)
                break; //Connection exists
        }
        if(i >= connections.size())
        {
            //Create new platform
            TrackConnectionItem newItem;
            newItem.info = info;
            newItem.visible = false;
            connections.append(newItem);
            i = connections.size() - 1;
        }

        TrackConnectionItem &item = connections[i];
        item.elements.append(elemPath);

        if(indexes)
            indexes->append(i);
    }

    return true;
}
