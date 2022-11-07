#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QDockWidget>

#include <QActionGroup>

#include <QFileDialog>
#include <QFile>

#include "manager/nodefindermgr.h"

#include <QMessageBox>

#include <QDebug>

EditorMainWindow::EditorMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    zoom(0),
    m_progMode(ProgramMode::NModes)
{
    ui->setupUi(this);

    nodeMgr = new NodeFinderMgr(this);

    setupActions();

    scrollArea = new QScrollArea(this);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setAlignment(Qt::AlignCenter);
    setCentralWidget(scrollArea);


    scrollArea->setWidget(nodeMgr->getCentralWidget(this));

    statusBar()->addPermanentWidget(nodeMgr->getStatusWidget(this));

    setProgramMode(ProgramMode::NoMode);
}

EditorMainWindow::~EditorMainWindow()
{
    delete ui;
}

void EditorMainWindow::loadXMLInEditor()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    QString(), QString(),
                                                    QString("XML (*.xml);;All Files (*)"));
    if(fileName.isEmpty())
        return;

    QFile f(fileName);
    if(!f.open(QFile::ReadOnly))
    {
        qDebug() << f.errorString();
        return;
    }

    nodeMgr->loadXML(&f);
}

void EditorMainWindow::loadSVGInEditor()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    QString(), QString(),
                                                    QString("SVG (*.svg);;All Files (*)"));
    if(fileName.isEmpty())
        return;

    QFile f(fileName);
    if(!f.open(QFile::ReadOnly))
    {
        qDebug() << f.errorString();
        return;
    }

    if(!nodeMgr->loadSVG(&f))
    {
        QMessageBox::warning(this, tr("Loading Error"), tr("Could not load SVG from '%1'").arg(fileName));
    }

    setProgramMode(ProgramMode::SVGMappingMode);
    setZoom(100);
}

void EditorMainWindow::saveConvertedSVG()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    QString(), QString(),
                                                    QString("SVG (*.svg);;All Files (*)"));
    if(fileName.isEmpty())
        return;

    QFile f(fileName);
    if(!f.open(QFile::WriteOnly | QFile::Truncate))
    {
        qDebug() << f.errorString();
        return;
    }

    nodeMgr->saveSVG(&f);
}

void EditorMainWindow::clearDocument()
{
    nodeMgr->clearDocument();

    setProgramMode(ProgramMode::NoMode);
}

void EditorMainWindow::setZoom(int val)
{
    if(val == zoom || val > 400 || val < 25)
        return;

    zoom = val;
    zoomSlider->setValue(zoom);
    zoomSpin->setValue(zoom);

    QSize s = scrollArea->widget()->sizeHint();
    s = s * zoom / 100;
    scrollArea->widget()->resize(s);
}

void EditorMainWindow::setupActions()
{
    //Creator
    QMenu *svgCreatorSubMenu = new QMenu(tr("Create SVG"));
    svgCreatorSubMenu->addAction(tr("From XML File"));
    svgCreatorSubMenu->addAction(tr("New Empty SVG"));

    QAction *creatorSaveSVG_act = new QAction(tr("Save To SVG"), this);
    m_creatorActions->addAction(creatorSaveSVG_act);

    //Editor
    QMenu *editorMenu = new QMenu(tr("Editor"), this);
    editorMenu->addAction(tr("Load XML"), this, &EditorMainWindow::loadXMLInEditor);
    editorMenu->addAction(tr("Unload XML"), nodeMgr, &NodeFinderMgr::clearXMLInEditor);
    m_editorActions->addAction(editorMenu->menuAction());
    editorMenu->addSeparator();

    auto addDockMode = [this, editorMenu](EditingModes mode)
    {
        QWidget *w = nodeMgr->getDockWidget(mode);
        QDockWidget *dockWidget = new QDockWidget(w->windowTitle());
        dockWidget->setWidget(w);
        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
        editorMenu->addAction(dockWidget->toggleViewAction());
    };

    addDockMode(EditingModes::LabelEditing);
    addDockMode(EditingModes::StationTrackEditing);
    addDockMode(EditingModes::TrackPathEditing);

    QAction *editorSaveSVG_act = new QAction(tr("Save SVG"), this);
    connect(editorSaveSVG_act, &QAction::triggered, this, &EditorMainWindow::saveConvertedSVG);
    m_editorActions->addAction(editorSaveSVG_act);

    //File
    QMenu *fileMenu = new QMenu(tr("File"), this);
    fileMenu->addAction(tr("Open SVG"), this, &EditorMainWindow::loadSVGInEditor);
    fileMenu->addAction(editorSaveSVG_act);
    fileMenu->addSeparator();
    fileMenu->addMenu(svgCreatorSubMenu);
    fileMenu->addAction(creatorSaveSVG_act);
    fileMenu->addSeparator();
    QAction *m_closeAction = fileMenu->addAction(tr("Close"), this, &EditorMainWindow::clearDocument);

    //View
    zoomSlider = new QSlider(Qt::Horizontal, this);
    zoomSlider->setRange(25, 400);
    zoomSlider->setToolTip(tr("Zoom"));
    connect(zoomSlider, &QSlider::valueChanged, this, &EditorMainWindow::setZoom);
    statusBar()->addPermanentWidget(zoomSlider);

    zoomSpin = new QSpinBox(this);
    zoomSpin->setRange(25, 400);
    zoomSpin->setSuffix(QChar('%'));
    connect(zoomSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &EditorMainWindow::setZoom);
    statusBar()->addPermanentWidget(zoomSpin);

    QMenu *viewMenu = new QMenu(tr("View"), this);
    m_viewActions->addAction(viewMenu->addAction(tr("Zoom In"), this, [this](){ setZoom(zoom - zoom%25 + 25); }));
    m_viewActions->addAction(viewMenu->addAction(tr("Zoom Out"), this, [this](){ setZoom(zoom - zoom%25 - 25); }));
    m_viewActions->addAction(m_closeAction);

    //Menubar
    ui->menubar->addMenu(fileMenu);
    ui->menubar->addMenu(editorMenu);
    ui->menubar->addMenu(viewMenu);
}

void EditorMainWindow::setProgramMode(ProgramMode mode)
{
    if(m_progMode == mode)
        return;

    m_progMode = mode;

    const bool enableViewActions = m_progMode != ProgramMode::NoMode;
    m_viewActions->setEnabled(enableViewActions);
    zoomSlider->setEnabled(enableViewActions);
    zoomSpin->setEnabled(enableViewActions);

    const bool enableCreatorActions = m_progMode != ProgramMode::SVGCreationMode;
    m_creatorActions->setEnabled(enableCreatorActions);

    const bool enableEditorActions = m_progMode != ProgramMode::SVGMappingMode;
    m_editorActions->setEnabled(enableEditorActions);
}
