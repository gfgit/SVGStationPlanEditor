#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "svgtinyconverter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QScrollArea;
class QSlider;
class QSpinBox;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadSVG();

public slots:
    void setZoom(int val);
    void setTrackPen(int val);

private slots:
    void onStationClicked(qint16 stationId, const QString &stationName, const QString &nodeName);

    void startConvert();
    void saveConvertedSVG();

private:
    void setSVGWidget(bool finder, QIODevice *dev);

private:
    Ui::MainWindow *ui;
    QWidget *svgWidget;
    QScrollArea *scrollArea;
    QSlider *zoomSlider;
    QSlider *trackPenSlider;
    QSpinBox *zoomSpin;
    int zoom;

    SVGTinyConverter conv;
};
#endif // MAINWINDOW_H
