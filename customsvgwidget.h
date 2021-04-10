#ifndef CUSTOMSVGWIDGET_H
#define CUSTOMSVGWIDGET_H

#include <QWidget>
#include <QVector>

class QSvgRenderer;
class QComboBox;

class CustomSVGWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CustomSVGWidget(QWidget *parent = nullptr);

    QSvgRenderer *renderer() const;

    QSize sizeHint() const override;

    bool event(QEvent *event) override;

    void loadStations();

    bool isEditable() const;
    void setEditable(bool value);

signals:
    void stationClicked(qint64 stationId, const QString& name, const QString& nodeName);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void locateLabels();

    void onEditorIdxChanged(int idx);
private:
    void openEditor();
    void closeEditor();
    void resizeEditor();

private:
    const double butWidth = 500.0;

    QSvgRenderer *mSvg;

    struct LabelEntry
    {
        qint64 nodeId;
        qint64 stationId;
        QString nodeName;
        QString stationName;
        QRectF rect;
    };
    QVector<LabelEntry> labels;
    QStringList stations;

    int highlightIdx;
    QComboBox *editor;
    bool editable;
};

#endif // CUSTOMSVGWIDGET_H
