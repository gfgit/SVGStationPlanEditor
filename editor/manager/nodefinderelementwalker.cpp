#include "nodefinderelementwalker.h"

NodeFinderElementWalker::NodeFinderElementWalker(const QStringList &orderedTags,
                                                 const QHash<QString, NodeFinderElementClass> &elementClasses) :
    m_orderedTags(orderedTags),
    m_elementClasses(elementClasses)
{

}

bool NodeFinderElementWalker::next()
{
    if(m_status.currentMap)
        m_status.iter++;

    while(!m_status.currentMap || m_status.iter == m_status.currentMap->end())
    {
        //Go to next class
        m_status.tagIdx++;
        m_status.currentMap = nullptr;

        if(m_status.tagIdx >= m_orderedTags.count())
        {
            m_status.tagIdx = m_orderedTags.count();
            if(m_status.tagIdx == 0)
                m_status.tagIdx = -1;
            return false; //It was last class
        }

        const QString tag = m_orderedTags.at(m_status.tagIdx);
        auto it = m_elementClasses.find(tag);
        if(it == m_elementClasses.end() || it.value().elements.isEmpty())
            continue; //Skip tag

        m_status.currentMap = &it.value().elements;
        m_status.iter = m_status.currentMap->begin();

        return true;
    }

    return m_status.currentMap != nullptr;
}

bool NodeFinderElementWalker::prev()
{
    while(!m_status.currentMap || m_status.iter == m_status.currentMap->begin())
    {
        //Go to previous class
        m_status.tagIdx--;
        m_status.currentMap = nullptr;

        if(m_status.tagIdx < 0)
        {
            m_status.tagIdx = -1;
            return false; //It was first class
        }

        const QString tag = m_orderedTags.at(m_status.tagIdx);
        auto it = m_elementClasses.find(tag);
        if(it == m_elementClasses.end() || it.value().elements.isEmpty())
            continue; //Skip tag

        m_status.currentMap = &it.value().elements;
        m_status.iter = std::prev(m_status.currentMap->end());

        return true;
    }

    if(m_status.currentMap)
        m_status.iter--;

    return m_status.currentMap != nullptr;
}

bool NodeFinderElementWalker::isValid() const
{
    return m_status.currentMap != nullptr;
}

QDomElement NodeFinderElementWalker::element()
{
    if(!m_status.currentMap)
        return QDomElement();

    return m_status.iter.value();
}
