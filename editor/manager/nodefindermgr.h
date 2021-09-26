#ifndef NODEFINDERMGR_H
#define NODEFINDERMGR_H

#include <QObject>

#include <QPointer>

#include <QPointF>
#include <QRectF>

#include "utils/nodefindereditingmodes.h"

class QIODevice;
class QWidget;
class NodeFinderSVGConverter;

class ItemBase;

class NodeFinderMgr : public QObject
{
    Q_OBJECT
public:
    explicit NodeFinderMgr(QObject *parent = nullptr);

    //Editing mode
    EditingModes mode() const;
    EditingSubModes getSubMode() const;

    //Widgets
    QWidget *getStatusWidget(QWidget *parent);
    QWidget *getCentralWidget(QWidget *parent);
    QWidget *getDockWidget(QWidget *parent);

    //Loading/Saving
    bool loadSVG(QIODevice *dev);
    bool saveSVG(QIODevice *dev);

    //For NodeFinderSVGWidget
    inline NodeFinderSVGConverter *getConverter() const { return converter; }
    inline bool shouldDrawLabels() const { return drawLabels; }
    inline bool shouldDrawStationTracks() const { return drawStationTracks; }

    //Track Pen
    inline int getTrackPenWidth() const { return trackPenWidth; }

    //Selection
    void startSelection(const QPointF& p);
    void endOrMoveSelection(const QPointF& p, bool isEnd);
    void clearSelection();
    inline bool isSelecting() const { return m_isSelecting; }
    inline QRectF getSelectionRect() const { return QRectF(selectionStart, selectionEnd).normalized(); }

signals:
    void modeChanged();
    void trackPenWidthChanged(int width);
    void repaintSVG();

public slots:
    //Track Pen
    void setTrackPenWidth(int value);

    void selectCurrentElem();
    void goToPrevElem();
    void goToNextElem();

    void requestAddSubElement();
    void requestRemoveSubElement();
    void requestEndEditItem();

    void clearCurrentItem();
    void requestEditItem(ItemBase *item, EditingModes m);

private:
    void setMode(EditingModes m, EditingSubModes sub = EditingSubModes::NotEditingCurrentItem);

private:
    EditingModes m_mode;
    EditingSubModes m_subMode;

    QPointer<QWidget> statusWidget;
    QPointer<QWidget> centralWidget;
    QPointer<QWidget> dockWidget;

    NodeFinderSVGConverter *converter;

    bool drawLabels;
    bool drawStationTracks;
    int trackPenWidth;

    QPointF selectionStart;
    QPointF selectionEnd;
    bool m_isSelecting;
};

#endif // NODEFINDERMGR_H
