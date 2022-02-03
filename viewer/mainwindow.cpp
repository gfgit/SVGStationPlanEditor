#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <ssplib/svgstationplanlib.h>
#include <QSvgRenderer>

#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QDockWidget>

#include <QFileDialog>
#include <QFile>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    stationPlan(nullptr),
    zoom(0)
{
    ui->setupUi(this);

    mSvg = new QSvgRenderer(this);

    stationPlan = new ssplib::StationPlan;
    viewer = new ssplib::SSPViewer(stationPlan);
    viewer->setRenderer(mSvg);

    scrollArea = new QScrollArea(this);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setAlignment(Qt::AlignCenter);
    setCentralWidget(scrollArea);
    scrollArea->setWidget(viewer);

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

    //statusBar()->addPermanentWidget(nodeMgr->getStatusWidget(this));

    QMenu *fileMenu = new QMenu(tr("File"), this);
    fileMenu->addAction(tr("Open SVG"), this, &MainWindow::loadSVG);
    ui->menubar->addMenu(fileMenu);

    QMenu *viewMenu = new QMenu(tr("View"), this);
    viewMenu->addAction(tr("Zoom In"), this, [this](){ setZoom(zoom - zoom%25 + 25); });
    viewMenu->addAction(tr("Zoom Out"), this, [this](){ setZoom(zoom - zoom%25 - 25); });
    viewMenu->addAction(tr("Zoom Fit"), this, &MainWindow::zoomToFit);
    ui->menubar->addMenu(viewMenu);

//    auto addDockMode = [this, viewMenu](EditingModes mode)
//    {
//        QWidget *w = nodeMgr->getDockWidget(mode);
//        QDockWidget *dockWidget = new QDockWidget(w->windowTitle());
//        dockWidget->setWidget(w);
//        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
//        viewMenu->addAction(dockWidget->toggleViewAction());
//    };

//    addDockMode(EditingModes::LabelEditing);
//    addDockMode(EditingModes::StationTrackEditing);
//    addDockMode(EditingModes::TrackPathEditing);
}

MainWindow::~MainWindow()
{
    delete ui;

    delete stationPlan;
    stationPlan = nullptr;
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

    ssplib::StreamParser parser(stationPlan, &f);
    if(!parser.parse())
    {
        qDebug() << "Parsing error";
        return;
    }

    f.reset();
    QXmlStreamReader xml(&f);
    if(!mSvg->load(&xml))
    {
        qDebug() << "SVG loading error";
        return;
    }

    setZoom(100);
    zoomToFit();
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

void MainWindow::zoomToFit()
{
    const QSize available = scrollArea->size();
    const QSize contents = scrollArea->widget()->sizeHint();

    const int zoomH = 100 * available.width() / contents.width();
    const int zoomV = 100 * available.height() / contents.height();

    const int val = qMin(zoomH, zoomV);
    setZoom(val);
}
