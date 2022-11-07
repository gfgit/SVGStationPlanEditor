#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "utils/nodefindereditingmodes.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QScrollArea;
class QSlider;
class QSpinBox;
class QActionGroup;
QT_END_NAMESPACE

class NodeFinderMgr;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void loadXMLInEditor();
    void loadSVGInEditor();
    void saveConvertedSVG();
    void clearDocument();
    void setZoom(int val);

private:
    void setupActions();

    void setProgramMode(ProgramMode mode);

private:
    Ui::MainWindow *ui;
    QScrollArea *scrollArea;
    QSlider *zoomSlider;
    QSpinBox *zoomSpin;
    int zoom;

    ProgramMode m_progMode;
    QActionGroup *m_viewActions;
    QActionGroup *m_creatorActions;
    QActionGroup *m_editorActions;

    NodeFinderMgr *nodeMgr;
};
#endif // MAINWINDOW_H
