#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "customsvgwidget.h"
#include "customsvgnodefinder.h"

#include <QSvgRenderer>

#include <QXmlStreamReader>
#include <QFile>
#include <QTemporaryFile>

#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>

#include <QMessageBox>

#include <QFileDialog>

#include "svgtinyconverter.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    svgWidget(nullptr),
    zoom(0)
{
    ui->setupUi(this);

    scrollArea = new QScrollArea(this);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setAlignment(Qt::AlignCenter);
    setCentralWidget(scrollArea);

    QMenu *fileMenu = new QMenu(tr("File"), this);
    fileMenu->addAction(tr("Open SVG"), this, &MainWindow::loadSVG);
    fileMenu->addAction(tr("Convert SVG"), this, &MainWindow::startConvert);
    ui->menubar->addMenu(fileMenu);

    QMenu *viewMenu = new QMenu(tr("View"), this);
    viewMenu->addAction(tr("Zoom In"), this, [this](){ setZoom(zoom - zoom%25 + 25); });
    viewMenu->addAction(tr("Zoom Out"), this, [this](){ setZoom(zoom - zoom%25 - 25); });
    viewMenu->addAction(tr("Finish Conversion"), this, &MainWindow::saveConvertedSVG);
    ui->menubar->addMenu(viewMenu);

    trackPenSlider = new QSlider(Qt::Horizontal, this);
    trackPenSlider->setRange(10, 100);
    trackPenSlider->setToolTip(tr("Track Pen Width"));
    connect(trackPenSlider, &QSlider::valueChanged, this, &MainWindow::setTrackPen);
    statusBar()->addPermanentWidget(trackPenSlider);

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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setSVGWidget(bool finder, QIODevice *dev)
{
    if(svgWidget)
    {
        scrollArea->setWidget(nullptr);
        delete  svgWidget;
        svgWidget = nullptr;
    }

    if(finder)
    {
        CustomSVGNodeFinder *w = new CustomSVGNodeFinder(&conv, this);
        w->load(dev);
        svgWidget = w;

        trackPenSlider->show();
        int val = w->getTrackPenWidth();
        trackPenSlider->setRange(qMax(10, val / 3), qMax(20, val * 3));
        trackPenSlider->setValue(val);
    }
    else
    {
        CustomSVGWidget *w = new CustomSVGWidget(this);
        QXmlStreamReader xml(dev);
        w->renderer()->load(&xml);
        w->setEditable(true);
        connect(w, &CustomSVGWidget::stationClicked, this, &MainWindow::onStationClicked);
        svgWidget = w;

        trackPenSlider->hide();
    }

    scrollArea->setWidget(svgWidget);
    svgWidget->adjustSize();
    setZoom(100);

    if(finder)
    {
        QMessageBox::information(this, tr("Select labels"),
                                 tr("Drag a selection with mouse left button to select a rectangle element.\n"
                                    "Then associate it with an Entry/Exit letter.\n"
                                    "It will be used to display the next station name connected to thet exit.\n"
                                    "When you are done go to View->Finish Conversion to save the modified SVG file."));
        setWindowTitle(tr("Editing"));
    }else{
        QMessageBox::information(this, tr("Let's play!"),
                                 tr("If the SVG doesn't render correctly first convert it (go to File->Convert SVG).\n"
                                    "Now you can chose the stations connected to each entry/exit:\n"
                                    "- Go with mouse on top of a label rectangle, the text will become red"
                                    "- Click in the small green rectangle, a drop down menu will appear, select a station."));
        setWindowTitle(tr("Station Plan"));
    }
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

    setSVGWidget(false, &f);
}

void MainWindow::startConvert()
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

    int but = QMessageBox::question(this, tr("Convert text?"),
                                    tr("Do you want to convert unsupported <tspan> elements?"));
    if(but == QMessageBox::Yes)
        conv.setConvertTspan(true);

    but = QMessageBox::question(this, tr("Process names?"),
                                tr("Do you want to process rects and groups to locate label areas?"));
    if(but == QMessageBox::Yes)
        conv.setProcessNames(true);

    if(!conv.getConvertTspan() && !conv.getProcessNames())
    {
        QMessageBox::warning(this, tr("Invalid paramenters"),
                             tr("You must chose at least one option or both.\n"
                                "You neither chose conversion neither processing of labels."));
        return;
    }

    if(!conv.load(&f, nullptr, nullptr, nullptr))
        return;

    f.close();

    if(!conv.convert())
        return;

    if(conv.getProcessNames())
    {
        QTemporaryFile tempFile;
        if(tempFile.open())
        {
            conv.write(&tempFile);
            tempFile.reset();

            setSVGWidget(true, &tempFile);
        }
    }
    else
    {
        saveConvertedSVG();
    }
}

void MainWindow::saveConvertedSVG()
{
    if(!conv.isActive())
        return;

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

    conv.setIDs();
    conv.write(&f);
    conv.clear();

    if(!qobject_cast<CustomSVGWidget *>(svgWidget))
    {
        f.close();
        if(!f.open(QFile::ReadOnly))
        {
            qDebug() << f.errorString();
        }
        setSVGWidget(false, &f);
    }
}

void MainWindow::setZoom(int val)
{
    if(val == zoom || val > 400 || val < 25)
        return;

    zoom = val;
    zoomSlider->setValue(zoom);
    zoomSpin->setValue(zoom);

    QSize s = svgWidget->sizeHint();
    s = s * zoom / 100;
    svgWidget->resize(s);
}

void MainWindow::setTrackPen(int val)
{
    CustomSVGNodeFinder *w = qobject_cast<CustomSVGNodeFinder *>(svgWidget);
    if(!w)
        return;
    w->setTrackPenWidth(val);
}

void MainWindow::onStationClicked(qint16 stationId, const QString& stationName, const QString& nodeName)
{
    if(stationName.isEmpty())
    {
        QMessageBox::information(this, tr("Not connected."),
                                 tr("Node <b>'%1'</b> is not connected to any station.<br>"
                                    "To connect it go on top with mouse, click green rectangle and choose a station from the drop down.")
                                 .arg(nodeName.at(nodeName.size() - 1)));
    }else{
        QMessageBox::information(this, stationName,
                                 tr("You clicked station \"%1\".\nID: %2")
                                 .arg(stationName).arg(stationId));
    }
}
