#include "customsvgwidget.h"

#include <QSvgRenderer>
#include <QPainter>

#include <QHelpEvent>
#include <QToolTip>

#include <QDebug>

#include <QComboBox>

CustomSVGWidget::CustomSVGWidget(QWidget *parent) :
    QWidget(parent),
    highlightIdx(-1),
    editor(nullptr),
    editable(true)
{
    mSvg = new QSvgRenderer(this);
    connect(mSvg, SIGNAL(repaintNeeded()), this, SLOT(locateLabels()));
    setBackgroundRole(QPalette::Light);
    setMouseTracking(true);
    loadStations();
}

QSvgRenderer *CustomSVGWidget::renderer() const
{
    return mSvg;
}

QSize CustomSVGWidget::sizeHint() const
{
    if(mSvg->isValid())
        return mSvg->defaultSize();
    return QSize(128, 64);
}

bool CustomSVGWidget::event(QEvent *e)
{
    if(e->type() == QEvent::ToolTip)
    {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(e);
        bool found = false;

        QRectF target = rect();
        QRectF source = mSvg->viewBoxF();

        const double inverseScaleFactor = source.width() / target.width();
        const QPointF pos = helpEvent->pos() * inverseScaleFactor + source.topLeft();

        for(const LabelEntry& entry : qAsConst(labels))
        {
            if(entry.rect.contains(pos))
            {
                found = true;
                QToolTip::showText(helpEvent->globalPos(), entry.stationName);
                break;
            }
        }
        if(!found)
        {
            QToolTip::hideText();
            e->ignore(); //See QToolTip documentation
        }

        return true;
    }
    return QWidget::event(e);
}

void CustomSVGWidget::paintEvent(QPaintEvent */*e*/)
{
    QRectF target = rect();
    QRectF source = mSvg->viewBoxF();

    QPainter p(this);
    mSvg->render(&p, target);

    const double scaleFactor = target.width() / source.width();

    QTransform transform;
    transform.scale(target.width() / source.width(),
                    target.height() / source.height());
    QRectF c2 = transform.mapRect(source);

    transform.reset();
    transform.translate(target.x() - c2.x(),
                        target.y() - c2.y());
    transform.scale(scaleFactor, scaleFactor);
    p.setTransform(transform);


    QFont f;
    f.setPointSize(350);
    p.setFont(f);

    QPen pen = p.pen();
    QPen highLightPen = pen;
    highLightPen.setColor(Qt::red);

    QFontMetrics metrics = p.fontMetrics();
    for(int i = 0; i < labels.size(); i++)
    {
        const LabelEntry& entry = labels.at(i);
        if(entry.rect.isNull())
            continue;

        QString text = entry.stationName;
        if(text.isEmpty())
            text = entry.nodeName;

        if(i == highlightIdx && !editor)
        {
            QRectF textRect = entry.rect.adjusted(butWidth, 0, 0, 0);
            QRectF butRect = entry.rect;
            butRect.setWidth(butWidth);

            p.fillRect(butRect, Qt::green);

            text = metrics.elidedText(text, Qt::ElideRight, textRect.width());
            p.setPen(highLightPen);
            p.drawText(entry.rect, text, QTextOption(Qt::AlignCenter));
            p.setPen(pen); //Reset to normal pen
        }
        else
        {
            text = metrics.elidedText(text, Qt::ElideRight, entry.rect.width());
            p.drawText(entry.rect, text, QTextOption(Qt::AlignCenter));
        }
    }
}

void CustomSVGWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(editor)
        return; //Do not move highlight while editing

    QRectF target = rect();
    QRectF source = mSvg->viewBoxF();

    const double inverseScaleFactor = source.width() / target.width();
    const QPointF pos = e->pos() * inverseScaleFactor + source.topLeft();

    int idx = -1;
    for(int i = 0; i < labels.size(); i++)
    {
        if(labels.at(i).rect.contains(pos))
        {
            idx = i;
            break;
        }
    }

    if(idx != highlightIdx)
    {
        highlightIdx = idx;
        update();
    }
}

void CustomSVGWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        QRectF target = rect();
        QRectF source = mSvg->viewBoxF();

        const double inverseScaleFactor = source.width() / target.width();
        const QPointF pos = e->pos() * inverseScaleFactor + source.topLeft();

        if(editor && highlightIdx != -1)
        {
            const LabelEntry& entry = labels.at(highlightIdx);
            if(!entry.rect.contains(pos))
                closeEditor();
        }

        if(editor)
            return; //Do not emit while editing

        for(int i = 0; i < labels.size(); i++)
        {
            const LabelEntry& entry = labels.at(i);
            QRectF textRect = entry.rect.adjusted(butWidth, 0, 0, 0);
            QRectF butRect = entry.rect;
            butRect.setWidth(butWidth);

            if(textRect.contains(pos))
            {
                emit stationClicked(entry.stationId, entry.stationName, entry.nodeName);
                break;
            }
            if(editable && butRect.contains(pos))
            {
                highlightIdx = i;
                openEditor();
                break;
            }
        }
    }
}

void CustomSVGWidget::resizeEvent(QResizeEvent */*e*/)
{
    resizeEditor();
}

void CustomSVGWidget::loadStations()
{
    stations.clear();
    stations.append({"Padova", "Venezia S.Lucia", "VERY_LONG_NAME_WITH_UNDERSCORES",
                     "Roma Termini", "Treviso C.Le", "Udine"});
}

void CustomSVGWidget::locateLabels()
{
    labels.clear();
    int id = 0;
    const QString fmt = QLatin1String("label_%1");
    for(QLatin1Char c('A'); c <= 'Z'; c = QLatin1Char(c.toLatin1() + 1))
    {
        const QString elem = fmt.arg(c);
        if(mSvg->elementExists(elem))
        {
            LabelEntry entry;
            entry.nodeId = 1;
            entry.stationId = id++;
            entry.nodeName = elem;
            entry.rect = mSvg->boundsOnElement(elem);
            labels.append(entry);
        }
    }

    update();
}

void CustomSVGWidget::openEditor()
{
    Q_ASSERT(!editor);
    if(!editable && highlightIdx == -1)
        return;

    editor = new QComboBox(this);
    connect(editor, QOverload<int>::of(&QComboBox::activated),
            this, &CustomSVGWidget::onEditorIdxChanged);

    editor->addItems(stations);
    editor->addItem("Not connected");
    editor->setItemData(editor->count() - 1, QBrush(Qt::blue), Qt::ForegroundRole);

    const LabelEntry& entry = labels.at(highlightIdx);
    int idx = editor->findText(entry.stationName);
    if(idx < 0)
        idx = editor->count() - 1;
    editor->setCurrentIndex(idx);

    resizeEditor();
    editor->show();
    editor->showPopup();
}

void CustomSVGWidget::closeEditor()
{
    Q_ASSERT(editor);
    delete editor;
    editor = nullptr;

    //Clear selection
    highlightIdx = -1;
    update();
}

void CustomSVGWidget::resizeEditor()
{
    if(!editor || highlightIdx == -1)
        return;

    const LabelEntry& entry = labels.at(highlightIdx);
    QRectF target = rect();
    QRectF source = mSvg->viewBoxF();

    const double scaleFactor = target.width() / source.width();

    QTransform transform;
    transform.scale(target.width() / source.width(),
                    target.height() / source.height());
    QRectF c2 = transform.mapRect(source);

    transform.reset();
    transform.translate(target.x() - c2.x(),
                        target.y() - c2.y());
    transform.scale(scaleFactor, scaleFactor);
    QRect r = transform.mapRect(entry.rect).toRect();
    editor->move(r.topLeft());
    editor->resize(r.size());

    QFont f = editor->font();
    f.setPointSize(400.0 * scaleFactor);
    editor->setFont(f);
}

bool CustomSVGWidget::isEditable() const
{
    return editable;
}

void CustomSVGWidget::setEditable(bool value)
{
    editable = value;
    if(!editable && editor)
        closeEditor();
}

void CustomSVGWidget::onEditorIdxChanged(int idx)
{
    if(!editor || highlightIdx == -1 || idx < 0)
        return;

    LabelEntry& entry = labels[highlightIdx];
    if(idx == editor->count() - 1)
        entry.stationName.clear(); // N/C, not connected node
    else
        entry.stationName = editor->itemText(idx);
}
