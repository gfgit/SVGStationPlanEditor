#ifndef NODEFINDERMGR_H
#define NODEFINDERMGR_H

#include <QObject>

#include <QPointer>

#include <QPointF>
#include <QRectF>

#include "utils/nodefindereditingmodes.h"

namespace ssplib {
class ItemBase;
} // namespace ssplib

class QIODevice;
class QWidget;
class NodeFinderSVGConverter;

class NodeFinderMgr : public QObject
{
    Q_OBJECT
public:
    explicit NodeFinderMgr(QObject *parent = nullptr);

    //Editing mode
    EditingModes mode() const;
    EditingSubModes getSubMode() const;
    QString getModeName(EditingModes mode) const;

    //Widgets
    QWidget *getStatusWidget(QWidget *parent);
    QWidget *getCentralWidget(QWidget *parent);
    QWidget *getDockWidget(EditingModes mode);

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

    void startElementSplitProcess();
    void triggerElementSplit(const QPointF &pos);

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
    void requestEditItem(ssplib::ItemBase *item, EditingModes m);

private:
    void setMode(EditingModes m, EditingSubModes sub = EditingSubModes::NotEditingCurrentItem);
    bool validateCurrentElement();

private:
    EditingModes m_mode;
    EditingSubModes m_subMode;

    QPointer<QWidget> statusWidget;
    QPointer<QWidget> centralWidget;

    NodeFinderSVGConverter *converter;

    bool drawLabels;
    bool drawStationTracks;
    int trackPenWidth;

    QPointF selectionStart;
    QPointF selectionEnd;
    bool m_isSelecting;
    bool m_isSinglePoint;
};

#endif // NODEFINDERMGR_H
