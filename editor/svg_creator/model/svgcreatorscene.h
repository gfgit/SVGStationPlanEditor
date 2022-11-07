#ifndef SVGCREATORSCENE_H
#define SVGCREATORSCENE_H

#include <QGraphicsScene>

class SvgCreatorManager;

class SvgCreatorScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit SvgCreatorScene(SvgCreatorManager *mgr, QObject *parent = nullptr);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *ev) override;

    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    SvgCreatorManager *manager;

    bool isDrawingLine = false;
    QPointF lineStart;
    QPointF lineEnd;
    QPointF lineRoundedEnd;
};

#endif // SVGCREATORSCENE_H
