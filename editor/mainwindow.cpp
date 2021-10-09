#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QDockWidget>

#include <QFileDialog>
#include <QFile>

#include "editor/manager/nodefindermgr.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    zoom(0)
{
    ui->setupUi(this);

    nodeMgr = new NodeFinderMgr(this);

    scrollArea = new QScrollArea(this);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setAlignment(Qt::AlignCenter);
    setCentralWidget(scrollArea);
    scrollArea->setWidget(nodeMgr->getCentralWidget(this));

    zoomSlider = new QSlider(Qt::Horizontal, this);
    zoomSlider->setRange(25, 400);
    zoomSlider->setToolTip(tr("Zoom"));
    connect(zoomSlider, &QSlider::valueChanged, this, &MainWindow::setZoom);
    statusBar()->addPermanentWidget(zoomSlider);

    zoomSpin = new QSpinBox(this);
    zoomSpin->setRange(25, 400);
    zoomSpin->setSuffix(QChar('%'));
    connect(zoomSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::setZoom);
    statusBar()->addPermanentWidget(zoomSpin);

    statusBar()->addPermanentWidget(nodeMgr->getStatusWidget(this));

    QMenu *fileMenu = new QMenu(tr("File"), this);
    fileMenu->addAction(tr("Open SVG"), this, &MainWindow::loadSVG);
    fileMenu->addAction(tr("Save SVG"), this, &MainWindow::saveConvertedSVG);
    ui->menubar->addMenu(fileMenu);

    QMenu *viewMenu = new QMenu(tr("View"), this);
    viewMenu->addAction(tr("Zoom In"), this, [this](){ setZoom(zoom - zoom%25 + 25); });
    viewMenu->addAction(tr("Zoom Out"), this, [this](){ setZoom(zoom - zoom%25 - 25); });
    ui->menubar->addMenu(viewMenu);

    auto addDockMode = [this, viewMenu](EditingModes mode)
    {
        QWidget *w = nodeMgr->getDockWidget(mode);
        QDockWidget *dockWidget = new QDockWidget(w->windowTitle());
        dockWidget->setWidget(w);
        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
        viewMenu->addAction(dockWidget->toggleViewAction());
    };

    addDockMode(EditingModes::LabelEditing);
    addDockMode(EditingModes::StationTrackEditing);
    addDockMode(EditingModes::TrackPathEditing);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadSVG()
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

    nodeMgr->loadSVG(&f);
    setZoom(100);
}

void MainWindow::saveConvertedSVG()
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

void MainWindow::setZoom(int val)
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
