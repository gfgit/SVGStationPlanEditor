#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScrollArea>
#include <QGraphicsView>

#include <QSlider>
#include <QSpinBox>
#include <QDockWidget>

#include <QActionGroup>

#include <QFileDialog>
#include <QFile>

#include "manager/nodefindermgr.h"
#include "svg_creator/svgcreatormanager.h"

#include <QMessageBox>

#include <QDebug>

EditorMainWindow::EditorMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_view(nullptr),
    zoom(0),
    m_progMode(ProgramMode::NModes),
    extraStatusWidget(nullptr)
{
    ui->setupUi(this);

    nodeMgr = new NodeFinderMgr(this);
    svgCreator = new SvgCreatorManager(this);
    connect(svgCreator, &SvgCreatorManager::splitTrackRequested, this, &EditorMainWindow::execSplit);

    scrollArea = new QScrollArea(this);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setAlignment(Qt::AlignCenter);
    setCentralWidget(scrollArea);

    m_view = new QGraphicsView(this);
    m_view->setMouseTracking(true);
    m_view->setBackgroundRole(QPalette::Dark);
    m_view->setAlignment(Qt::AlignCenter);
    m_view->hide();
    m_view->setScene(svgCreator->getScene());

    setupActions();
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
    svgCreator->clear();

    setProgramMode(ProgramMode::NoMode);
}

void EditorMainWindow::setZoom(int val)
{
    if(val == zoom || val > 400 || val < 25)
        return;

    zoom = val;
    zoomSlider->setValue(zoom);
    zoomSpin->setValue(zoom);

    QWidget *contentWidget = scrollArea->widget();
    if(contentWidget)
    {
        QSizeF s = contentWidget->sizeHint();
        s = s * double(zoom) / 100.0;
        contentWidget->resize(s.toSize());
    }

    m_view->setTransform(QTransform::fromScale(val / 100.0, val / 100.0));
}

void EditorMainWindow::createSVGFromFile()
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

    if(!svgCreator->loadStationXML(&f))
    {

    }

    setProgramMode(ProgramMode::SVGCreationMode);
}

void EditorMainWindow::execSplit(TrackConnectionItem *item, bool silent)
{
    SvgTrackItemSplitter splitter(svgCreator);
    splitter.setItem(item);

    if(splitter.getIntersectionCount() == 0)
    {
        if(!silent)
        {
            //Tell the user
            QMessageBox::information(this, tr("Track Split"),
                                     tr("Selected track has no intersections to split."));
        }
        return;
    }

    bool skip = false;

    QPointer<QMessageBox> msgBox = new QMessageBox(this);
    msgBox->setIcon(QMessageBox::Question);
    msgBox->addButton(QMessageBox::Cancel);
    msgBox->addButton(tr("Split"), QMessageBox::YesRole);
    msgBox->addButton(tr("Skip"), QMessageBox::NoRole);

    QString text = tr("Split intersection (%2/%1).\n"
                      "To make a diamond crossing press Skip.\n"
                      "To make a tournout or double-slip switch press Split.")
                       .arg(splitter.getIntersectionCount());

    do {
        msgBox->setText(text.arg(splitter.getCurrentIndex() + 1)); //Make 1-based index
        msgBox->exec();
        if(!msgBox)
            break;

        if(msgBox->standardButton(msgBox->clickedButton()) == QMessageBox::Cancel)
            break;

        int role = msgBox->buttonRole(msgBox->clickedButton());
        skip = (role == QMessageBox::NoRole);
    }
    while(splitter.applyIntersection(skip));

    delete msgBox;
}

void EditorMainWindow::setupActions()
{
    m_viewActions = new QActionGroup(this);
    m_creatorActions = new QActionGroup(this);
    m_editorActions = new QActionGroup(this);

    //Creator
    QMenu *svgCreatorSubMenu = new QMenu(tr("Create SVG"));
    svgCreatorSubMenu->addAction(tr("From XML File"), this, &EditorMainWindow::createSVGFromFile);
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

    //addDockMode(EditingModes::LabelEditing);
    //addDockMode(EditingModes::StationTrackEditing);
    //addDockMode(EditingModes::TrackPathEditing);

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

    const bool enableCreatorActions = m_progMode == ProgramMode::SVGCreationMode;
    m_creatorActions->setEnabled(enableCreatorActions);

    const bool enableEditorActions = m_progMode == ProgramMode::SVGMappingMode;
    m_editorActions->setEnabled(enableEditorActions);

    QWidget *currentWidget = scrollArea->takeWidget();
    if(currentWidget)
    {
        currentWidget->setParent(this);
        currentWidget->hide();
    }

    currentWidget = centralWidget();
    if(currentWidget)
    {
        takeCentralWidget();
        currentWidget->hide();
    }

    if(extraStatusWidget)
    {
        statusBar()->removeWidget(extraStatusWidget);
        extraStatusWidget->hide();
        extraStatusWidget = nullptr;
    }

    if(enableEditorActions)
    {
        setCentralWidget(scrollArea);
        scrollArea->show();

        currentWidget = nodeMgr->getCentralWidget(this);
        scrollArea->setWidget(currentWidget);

        extraStatusWidget = nodeMgr->getStatusWidget(this);
        statusBar()->addPermanentWidget(extraStatusWidget);
        extraStatusWidget->show();
    }
    else if(enableCreatorActions)
    {
        setCentralWidget(m_view);
        m_view->show();
    }

    setZoom(100);
}
