#ifndef IEDITOR_H
#define IEDITOR_H

#include <QObject>

class EditorMainWindow;

class IEditor : public QObject
{
    Q_OBJECT
public:
    explicit IEditor(QObject *parent = nullptr);

//FIXME: implement generic interface for SvgCreatorManager and NodeFinderManager
};

#endif // IEDITOR_H
