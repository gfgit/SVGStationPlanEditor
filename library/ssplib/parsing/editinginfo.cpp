#include "editinginfo.h"

#include "ssplib/utils/svg_constants.h"

#ifdef SSPLIB_ENABLE_EDITING

using namespace ssplib;

EditingInfo::EditingInfo() :
    generatedIdSerial(0),
    generatedIdBase(QLatin1String("generated_"))
{

}

void EditingInfo::storeElement(QDomElement &e)
{
    if(e.hasAttribute(ssplib::svg_attr::ID))
    {
        QString id = e.attribute(ssplib::svg_attr::ID);
        if(namedElements.contains(id))
        {
            //Duplicate id, rename element
            id = getFreeId_internal(generatedIdBase, generatedIdSerial);
            if(id.isEmpty())
                e.removeAttribute(ssplib::svg_attr::ID);
            else
                e.setAttribute(ssplib::svg_attr::ID, id);
        }
        namedElements.insert(id, e);
    }

    //TODO: store
}

void EditingInfo::clear()
{
    resetIdGenerator();

    namedElements.clear();
}

void EditingInfo::resetIdGenerator()
{
    generatedIdSerial = 0;
}

QString EditingInfo::getFreeId_internal(const QString &base, int &counter)
{
    const QString fmt = base + QLatin1String("%1");
    for(int i = 0; i < 2000; i++)
    {
        const QString id = fmt.arg(counter++);
        if(!namedElements.contains(id))
            return id;
    }
    return QString();
}

#endif // SSPLIB_ENABLE_EDITING
