#ifndef SVGCREATORSCENE_H
#define SVGCREATORSCENE_H

#include <QGraphicsScene>

class SvgCreatorManager;

class SvgCreatorScene : public QGraphicsScene
{
    Q_OBJECT
public:
    enum class ToolMode
    {
        MoveItems = 0,
        DrawTracks,
        NTools
    };

    explicit SvgCreatorScene(SvgCreatorManager *mgr, QObject *parent = nullptr);

    void setOverlayLines(const QLineF& lineA, const QLineF& lineB);
    void clearOverlayLines();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *ev) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev) override;

    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    void addTrackConnection(const QLineF& line);

    bool snapToPoint(QPointF &pos, const QPointF &startPos);

private:
    SvgCreatorManager *manager;

    ToolMode m_toolMode = ToolMode::DrawTracks;

    bool isDrawingLine = false;
    QPointF lineStart;
    QPointF lineEnd;
    QPointF lineStartSnapped;
    QPointF lineRoundedEnd;

    QLineF overlayLineA;
    QLineF overlayLineB;
    bool hasOverlayLines = false;
};

#endif // SVGCREATORSCENE_H
