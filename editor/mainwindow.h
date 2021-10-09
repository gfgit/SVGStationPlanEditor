#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QScrollArea;
class QSlider;
class QSpinBox;
QT_END_NAMESPACE

class NodeFinderMgr;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void loadSVG();
    void saveConvertedSVG();
    void setZoom(int val);

private:
    Ui::MainWindow *ui;
    QScrollArea *scrollArea;
    QSlider *zoomSlider;
    QSpinBox *zoomSpin;
    int zoom;

    NodeFinderMgr *nodeMgr;
};
#endif // MAINWINDOW_H
