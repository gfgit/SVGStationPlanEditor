#ifndef NODEFINDERMGR_H
#define NODEFINDERMGR_H

#include <QObject>

#include <QPointer>

class QIODevice;
class QWidget;
class NodeFinderSVGConverter;

class NodeFinderMgr : public QObject
{
    Q_OBJECT
public:
    explicit NodeFinderMgr(QObject *parent = nullptr);

    //Editing mode
    enum class EditingModes
    {
        LabelEditing = 0,
        StationTrackEditing,
        TrackPathEditing,
        NModes
    };

    EditingModes mode() const;
    void setMode(const EditingModes &mode);

    //Widgets
    QWidget *getStatusWidget(QWidget *parent);
    QWidget *getCentralWidget(QWidget *parent);
    QWidget *getDockWidget(QWidget *parent);

    //Loading/Saving
    bool loadSVG(QIODevice *dev);
    bool saveSVG(QIODevice *dev);

signals:
    void modeChanged(int mode);

public slots:
    void selectCurrentElem();
    void goToNextElem();

private:
    EditingModes m_mode;

    QPointer<QWidget> statusWidget;
    QPointer<QWidget> centralWidget;
    QPointer<QWidget> dockWidget;

    NodeFinderSVGConverter *converter;
};

#endif // NODEFINDERMGR_H
