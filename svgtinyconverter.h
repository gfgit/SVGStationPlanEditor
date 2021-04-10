#ifndef SVGTINYCONVERTER_H
#define SVGTINYCONVERTER_H

#include <QHash>
#include <QDomDocument>
#include <QRectF>

#include <QPainterPath>

#include "elementclass.h"

class QIODevice;

//TODO: some <font> elements produced by SVG Optimizers do not have ID attribute, Qt SVG complains, add attribute

class SVGTinyConverter
{
public:
    SVGTinyConverter();

    bool load(QIODevice *source, QString *errMsg, int *errLine, int *errCol);
    bool convert();
    bool setIDs();
    bool write(QIODevice *output);
    void clear();

    void setProcessNames(bool value);
    inline bool getProcessNames() const { return processNames; }

    void setConvertTspan(bool value);
    inline bool getConvertTspan() const { return convertTspan; }

    bool isActive() const;

    QDomElement elementById(const QString& id);

    bool walkElements(const QStringList& classPrecedence, QDomElement &result, ElementClass::Callback fun) const;

    bool isElementUsedAsLabel(const QDomElement &e, QChar &letterOut) const;

private:
    QDomElement elementByIdRecursive(QDomElement e, const QString &id);

    void processDefs(QDomElement &g);
    void processGroup(QDomElement &g);
    void processText(QDomElement &text);
    void processTspan(QDomElement &tspan, QDomElement &text);
    void processInternalTspan(QDomElement &top, QDomElement &cur, QString &value);

    inline QString getFreeId(const QString& base)
    {
        int counter = 0;
        return getFreeId_internal(base, counter);
    }

private:
    friend class ElementClass;
    QString getFreeId_internal(const QString& base, int &counter);
    static const QString idAttr;

    void renameElement(QDomElement &e, const QString &newId);

    void storeElement(QDomElement e);

    inline void registerClass(const QString& tagName)
    {
        ElementClass c(this, tagName, tagName + '_');
        elementClasses.insert(tagName, c);
    }

private:
    friend class CustomSVGNodeFinder;

    bool active;
    bool processNames;
    bool convertTspan;

    QDomDocument mDoc;

    ElementClass::ElementHash fakeIds;
    QHash<QString, ElementClass> elementClasses;

    struct LabelEntry
    {
        QChar nodeName;
        QDomElement elem;
        QRectF rect;
    };

    QHash<QChar, LabelEntry> labels;

    struct TrackEntry
    {
        QString trackName;
        int trackPos;
        QDomElement elem;
        QPainterPath path;
    };

    QHash<QString, TrackEntry> tracks;

    struct TrackPathEntry
    {
        QChar gateLetter;
        int gateTrack;
        int stationTrackPos;
        QDomElement elem;
        QPainterPath path;
    };

    QHash<QString, TrackPathEntry> trackPaths;
};

#endif // SVGTINYCONVERTER_H
