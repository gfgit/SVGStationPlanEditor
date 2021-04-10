#ifndef CUSTOMSVGNODEFINDER_H
#define CUSTOMSVGNODEFINDER_H

#include <QWidget>

#include <QHash>

#include <QPainterPath>

#include "svgtinyconverter.h"

class QSvgRenderer;

class CustomSVGNodeFinder : public QWidget
{
    Q_OBJECT
public:
    enum Mode
    {
        NoSelection,
        SelectLabel,
        SelectTrack
    };


    CustomSVGNodeFinder(SVGTinyConverter *c, QWidget *parent = nullptr);

    QSize sizeHint() const override;

    bool load(QIODevice *dev);

    void setTrackPenWidth(int value);

    int getTrackPenWidth() const;

protected:
    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private slots:
    void locateElements();

private:
    void locateLabels();
    void removeLabel(QChar ch);
    Mode askModeToUser();
    void addNewLabel(const QDomElement &res, const QRectF &bounds);
    void addNewTrack(const QDomElement &res);

private:
    SVGTinyConverter *conv;
    QSvgRenderer *mSvg;
    QPoint start;
    QPoint end;
    bool isSelecting;

    QPainterPath trackPath;
    bool isTrackSelected;

    int trackPenWidth;
};

#endif // CUSTOMSVGNODEFINDER_H
