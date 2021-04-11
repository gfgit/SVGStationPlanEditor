#include "nodefinderelementwalker.h"

NodeFinderElementWalker::NodeFinderElementWalker(const QStringList &orderedTags,
                                                 const QHash<QString, NodeFinderElementClass> &elementClasses) :
    m_orderedTags(orderedTags),
    m_elementClasses(elementClasses),
    m_currentMap(nullptr),
    m_tagIdx(-1)
{

}

bool NodeFinderElementWalker::next()
{
    if(m_currentMap)
        m_iter++;

    while(!m_currentMap || m_iter == m_currentMap->end())
    {
        //Go to next class
        m_tagIdx++;
        m_currentMap = nullptr;

        if(m_tagIdx >= m_orderedTags.count())
        {
            m_tagIdx = m_orderedTags.count();
            if(m_tagIdx == 0)
                m_tagIdx = -1;
            return false; //It was last class
        }

        const QString tag = m_orderedTags.at(m_tagIdx);
        auto it = m_elementClasses.find(tag);
        if(it == m_elementClasses.end() || it.value().elements.isEmpty())
            continue; //Skip tag

        m_currentMap = &it.value().elements;
        m_iter = m_currentMap->begin();

        return true;
    }

    return m_currentMap != nullptr;
}

bool NodeFinderElementWalker::prev()
{
    while(!m_currentMap || m_iter == m_currentMap->begin())
    {
        //Go to previous class
        m_tagIdx--;
        m_currentMap = nullptr;

        if(m_tagIdx < 0)
        {
            m_tagIdx = -1;
            return false; //It was first class
        }

        const QString tag = m_orderedTags.at(m_tagIdx);
        auto it = m_elementClasses.find(tag);
        if(it == m_elementClasses.end() || it.value().elements.isEmpty())
            continue; //Skip tag

        m_currentMap = &it.value().elements;
        m_iter = m_currentMap->end() - 1;

        return true;
    }

    if(m_currentMap)
        m_iter--;

    return m_currentMap != nullptr;
}

bool NodeFinderElementWalker::isValid() const
{
    return m_currentMap != nullptr;
}

QDomElement NodeFinderElementWalker::element()
{
    if(!m_currentMap)
        return QDomElement();

    return m_iter.value();
}
