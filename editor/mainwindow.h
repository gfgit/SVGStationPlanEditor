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
class QGraphicsView;
QT_END_NAMESPACE

class NodeFinderMgr;
class SvgCreatorManager;
class TrackConnectionItem;

class EditorMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    EditorMainWindow(QWidget *parent = nullptr);
    ~EditorMainWindow();

public slots:
    void loadXMLInEditor();
    void loadSVGInEditor();
    void saveConvertedSVG();
    void clearDocument();
    void setZoom(int val);

    void createSVGFromFile();

    void execSplit(TrackConnectionItem *item, bool silent);

private:
    void setupActions();

    void setProgramMode(ProgramMode mode);

private:
    Ui::MainWindow *ui;
    QScrollArea *scrollArea;
    QGraphicsView *m_view;
    QSlider *zoomSlider;
    QSpinBox *zoomSpin;
    int zoom;

    ProgramMode m_progMode;
    QActionGroup *m_viewActions;
    QActionGroup *m_creatorActions;
    QActionGroup *m_editorActions;

    NodeFinderMgr *nodeMgr;
    SvgCreatorManager *svgCreator;
    QWidget *extraStatusWidget;
};
#endif // MAINWINDOW_H
