#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QScrollArea;
class QSlider;
class QSpinBox;
class QSvgRenderer;
QT_END_NAMESPACE

namespace ssplib {
class SSPViewer;
class StationPlan;
} // namespace ssplib

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void loadSVG();
    void setZoom(int val);
    void zoomToFit();

private: 
    void loadSVG_internal(QIODevice *dev);

private:
    Ui::MainWindow *ui;
    QScrollArea *scrollArea;
    QSlider *zoomSlider;
    QSpinBox *zoomSpin;
    ssplib::SSPViewer *viewer;

private:
    QSvgRenderer *mSvg;
    ssplib::StationPlan *stationPlan;
    int zoom;
};
#endif // MAINWINDOW_H
