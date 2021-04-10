#include "customsvgnodefinder.h"

#include <QSvgRenderer>
#include <QXmlStreamReader>

#include <QMouseEvent>

#include <QPainter>
#include <QPainterPath>

#include <QMessageBox>
#include <QPushButton>

#include <QInputDialog>

#include <QPointer>

#include "svgutils.h"

CustomSVGNodeFinder::CustomSVGNodeFinder(SVGTinyConverter *c, QWidget *parent) :
    QWidget(parent),
    conv(c),
    isSelecting(false),
    isTrackSelected(false),
    trackPenWidth(10)
{
    mSvg = new QSvgRenderer(this);
    setBackgroundRole(QPalette::Light);
}

QSize CustomSVGNodeFinder::sizeHint() const
{
    if(mSvg->isValid())
        return mSvg->defaultSize();
    return QSize(128, 64);
}

bool CustomSVGNodeFinder::load(QIODevice *dev)
{
    QXmlStreamReader reader(dev);
    if(!mSvg->load(&reader))
        return false;
    isTrackSelected = false; //Clear track
    QSize sz = mSvg->viewBox().size();
    trackPenWidth = qMin(sz.width(), sz.height()) / 100;
    if(trackPenWidth < 10)
        trackPenWidth = 10;
    locateLabels();
    return true;
}

void CustomSVGNodeFinder::paintEvent(QPaintEvent */*e*/)
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

    const QString fmt = QLatin1String("label_%1");
    QColor color(Qt::blue);
    color.setAlpha(100);

    //QFontMetrics metrics = p.fontMetrics();
    for(const SVGTinyConverter::LabelEntry& entry : qAsConst(conv->labels))
    {
        if(entry.rect.isNull())
            continue;

        p.fillRect(entry.rect, color);

        QString text = fmt.arg(entry.nodeName);
        //text = metrics.elidedText(text, Qt::ElideRight, entry.rect.width());
        p.drawText(entry.rect, text, QTextOption(Qt::AlignCenter));
    }

    if(isTrackSelected)
    {
        QPen trackPen(Qt::darkGreen, trackPenWidth);
        p.setPen(trackPen);
        p.drawPath(trackPath);
    }

    //Selection is in normal coordinates so reset scaling
    p.resetTransform();
    if(start != end)
    {
        QRect selection(start, end);
        QColor col(isSelecting ? Qt::red : Qt::green);
        col.setAlpha(50);
        p.fillRect(selection, col);
    }
}

void CustomSVGNodeFinder::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        QRectF target = rect();
        QRectF source = mSvg->viewBoxF();

        const double inverseScaleFactor = source.width() / target.width();
        const QPointF pos = e->pos() * inverseScaleFactor + source.topLeft();

        QChar ch;
        for(const SVGTinyConverter::LabelEntry& entry : qAsConst(conv->labels))
        {
            if(entry.rect.contains(pos))
            {
                ch = entry.nodeName;
                break;
            }
        }

        if(ch.isNull())
        {
            //Start selection
            isSelecting = true;
            start = end = e->pos();
        }
        else
        {
            int but = QMessageBox::question(this, tr("Remove label?"),
                                            tr("Remove label '%1'?").arg(ch));
            if(but == QMessageBox::Yes)
            {
                removeLabel(ch);
            }

            start = end = QPoint();
            isSelecting = false;
        }
    }
    else
    {
        start = end = QPoint();
        isSelecting = false;
    }
    update();
}

void CustomSVGNodeFinder::mouseMoveEvent(QMouseEvent *e)
{
    if(isSelecting)
    {
        //Update selection rectangle overlay
        end = e->pos();
        update();
    }
}

void CustomSVGNodeFinder::mouseReleaseEvent(QMouseEvent *e)
{
    if(isSelecting)
    {
        end = e->pos();
        isSelecting = false;
        update();
        QMetaObject::invokeMethod(this, &CustomSVGNodeFinder::locateElements);
    }
}

void CustomSVGNodeFinder::locateLabels()
{
    //Find already existing labels before editing
    const QString fmt = QLatin1String("label_%1");
    for(QLatin1Char c('A'); c <= 'Z'; c = QLatin1Char(c.toLatin1() + 1))
    {
        QString id = fmt.arg(c);
        SVGTinyConverter::LabelEntry entry;
        entry.nodeName = c;
        entry.elem = conv->elementById(id);
        if(entry.elem.isNull())
            continue;

        //The label already exists
        if(!mSvg->elementExists(id))
        {
            //This element is not visible so it cannot be chosen as label
            //Try to replace its ID to something else

            const QString newId = conv->getFreeId(id + QLatin1String("_old"));
            conv->renameElement(entry.elem, newId);

            //Then skip this item
            continue;
        }

        entry.rect = mSvg->boundsOnElement(id);
        conv->labels.insert(c, entry);
    }

    update();
}

void CustomSVGNodeFinder::locateElements()
{
    //Try to locate <rect> contained inside current selection rectangle
    //If not found look for a <g> element (group)
    //If again not found warn the user that the selected area is empty
    //(there's no usable element inside it)

    if(isSelecting || start == end)
        return;

    //Ask mode
    Mode mode = askModeToUser();
    if(mode == NoSelection)
        return;

    QRectF target = rect();
    QRectF source = mSvg->viewBoxF();

    const double inverseScaleFactor = source.width() / target.width();
    const QPointF topLeft = start * inverseScaleFactor + source.topLeft();
    const QPointF bottomRight = end * inverseScaleFactor + source.topLeft();

    //Normalize rect in case top is below bottom
    QRectF selection = QRectF(topLeft, bottomRight).normalized();
    QRectF bounds;

    QStringList tagPrecedence;
    if(mode == SelectLabel)
    {
        tagPrecedence << "rect" << "path" << "group";
    }
    else
    {
        //Tracks can be line, path or polyline
        tagPrecedence << "line" << "path" << "polyline";
    }

    auto fun = [&, this](const QDomElement &e) -> ElementClass::CallbackResult
    {
        const QString id = e.attribute(SVGTinyConverter::idAttr);
        bounds = mSvg->boundsOnElement(id);
        if(!selection.contains(bounds))
        {
            return ElementClass::CallbackResult::KeepSearching;
        }

        //Restrict selection to element bounds
        start = bounds.topLeft().toPoint();
        end = bounds.bottomRight().toPoint();

        //Clear current track
        bool wasTrackSelected = isTrackSelected;
        QPainterPath oldTrackPath = trackPath;

        if(mode == SelectTrack)
        {
            if(!utils::convertElementToPath(e, trackPath))
            {
                QMessageBox::warning(this, tr("Error"),
                                     tr("Cannot parse element.\n"
                                        "NOTE: supperted types are <line>, <polyline>\n"
                                        "and <path> (limited to M,m,L,l)"));
                //Restore track
                isTrackSelected = wasTrackSelected;
                trackPath = oldTrackPath;
                return ElementClass::CallbackResult::KeepSearching;
            }

            //Show our new track
            isTrackSelected = true;
        }

        //Force repaint before showing message box
        update();

        int but = QMessageBox::question(this, tr("Select element?"),
                                        tr("Found element '%1'. Select it?")
                                            .arg(e.attribute(SVGTinyConverter::idAttr)),
                                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort);

        //Restore track
        isTrackSelected = wasTrackSelected;
        trackPath = oldTrackPath;

        if(but == QMessageBox::Abort)
        {
            return ElementClass::CallbackResult::AbortSearch;
        }
        if(but == QMessageBox::No)
        {
            return ElementClass::CallbackResult::KeepSearching;
        }

        QChar labelLetter;
        if(conv->isElementUsedAsLabel(e, labelLetter))
        {
            QMessageBox::warning(this, tr("Already used"),
                                 tr("Element '%1' is already used by entry '%2'.\n"
                                    "If you want to re-assign it, first remove it by clicking on it.")
                                     .arg(e.attribute(SVGTinyConverter::idAttr)).arg(labelLetter));
            return ElementClass::CallbackResult::KeepSearching;
        }

        return ElementClass::CallbackResult::ReturnCurrentElement;
    };

    QDomElement res;

    if(!conv->walkElements(tagPrecedence, res, fun))
    {
        QMessageBox::warning(this, tr("Empty selection"),
                             tr("Cannot find elements contained in selection"));
        start = end = QPoint(); //Clear selection
        update();
        return;
    }

    if(mode == SelectLabel)
    {
        addNewLabel(res, bounds);
    }
    else
    {
        addNewTrack(res);
    }

    update();
}

void CustomSVGNodeFinder::removeLabel(QChar ch)
{
    auto entry = conv->labels.find(ch);
    if(entry == conv->labels.end())
        return; //Not existing

    QDomElement el = entry->elem;
    QString id = QLatin1String("label_%1");
    id = id.arg(ch);
    if(el.attribute(SVGTinyConverter::idAttr) == id)
    {
        //The element must be renamed to free this name
        //for later use in another label
        const QString newId = conv->getFreeId(id + QLatin1String("_old"));
        conv->renameElement(entry->elem, newId);
    }

    //Remove
    conv->labels.erase(entry);
}

CustomSVGNodeFinder::Mode CustomSVGNodeFinder::askModeToUser()
{
    QPointer<QMessageBox> msgBox(new QMessageBox(this));
    msgBox->setIcon(QMessageBox::Question);
    msgBox->setWindowTitle(tr("Mode"));
    msgBox->setText(tr("Do you want to select LABELS or TRACKS?"));
    QPushButton *labelBut = msgBox->addButton(tr("Labels"), QMessageBox::AcceptRole);
    QPushButton *trackBut = msgBox->addButton(tr("Tracks"), QMessageBox::AcceptRole);
    msgBox->addButton(QMessageBox::Cancel);

    msgBox->setDefaultButton(labelBut);
    msgBox->exec();
    if(!msgBox)
        return NoSelection;

    QAbstractButton *clicked = msgBox->clickedButton();
    if(clicked == labelBut)
        return SelectLabel;
    if(clicked == trackBut)
        return SelectTrack;
    return NoSelection;
}

void CustomSVGNodeFinder::addNewLabel(const QDomElement& res, const QRectF& bounds)
{
    QChar ch;
    QString str;
    while (str.isEmpty())
    {
        str = QInputDialog::getText(this, tr("Entry Name"),
                                    tr("Insert a letter to name this entry. Leave empty to cancel"));
        if(str.isEmpty())
        {
            int but = QMessageBox::question(this, tr("Cancel selection?"),
                                        tr("Do you want to cancel?"));
            if(but == QMessageBox::Yes)
                break;
            continue;
        }
        ch = str.at(0).toUpper();
        if(!ch.isLetter())
        {
            QMessageBox::warning(this, tr("Invalid character"), tr("The name must be a single letter."));
            str.clear();
            continue;
        }
        if(conv->labels.contains(ch))
        {
            int but = QMessageBox::question(this, tr("Replace?"),
                                        tr("Entry '%1' already exists. Replace it?").arg(ch));
            if(but == QMessageBox::Yes)
                break;
            str.clear();
            continue;
        }
    }

    if(str.isEmpty())
    {
        //Canceled by user.
        return;
    }

    //Remove previous
    removeLabel(ch);

    SVGTinyConverter::LabelEntry entry;
    entry.nodeName = ch;
    entry.elem = res;
    entry.rect = bounds;

    conv->labels.insert(ch, entry);
}

void CustomSVGNodeFinder::addNewTrack(const QDomElement& res)
{
    isTrackSelected = false; //Clear track
    QPainterPath path;
    if(!utils::convertElementToPath(res, path))
    {
        QMessageBox::warning(this, tr("Error"),
                             tr("Cannot parse element.\n"
                                "NOTE: supperted types are <line>, <polyline>\n"
                                "and <path> (limited to M,m,L,l)"));
        return;
    }

    trackPath = path;
    isTrackSelected = true;
}

int CustomSVGNodeFinder::getTrackPenWidth() const
{
    return trackPenWidth;
}

void CustomSVGNodeFinder::setTrackPenWidth(int value)
{
    if(trackPenWidth == value)
        return;
    trackPenWidth = value;
    update();
}
