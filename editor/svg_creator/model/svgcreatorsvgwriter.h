#ifndef SVGCREATORSVGWRITER_H
#define SVGCREATORSVGWRITER_H

class QIODevice;
class QXmlStreamWriter;

class SvgCreatorManager;
class TrackConnectionItem;
class PlatformItem;
class GateItem;

class SvgCreatorSVGWriter
{
public:
    SvgCreatorSVGWriter(SvgCreatorManager *mgr);

    bool writeSVG(QIODevice *dev);

private:
    void writeTrackConn(QXmlStreamWriter &xml, const TrackConnectionItem *item);
    void writePlatform(QXmlStreamWriter &xml, const PlatformItem *item);
    void writeGate(QXmlStreamWriter &xml, const GateItem *item);
    void writeStLabel(QXmlStreamWriter &xml);

private:
    SvgCreatorManager *manager;
};

#endif // SVGCREATORSVGWRITER_H
