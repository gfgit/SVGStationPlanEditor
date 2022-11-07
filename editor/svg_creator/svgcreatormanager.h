#ifndef SVGCREATORMANAGER_H
#define SVGCREATORMANAGER_H

#include <QObject>

class SvgCreatorScene;

class SvgCreatorManager : public QObject
{
    Q_OBJECT
public:
    explicit SvgCreatorManager(QObject *parent = nullptr);

signals:

};

#endif // SVGCREATORMANAGER_H
