#include "nodefindermgr.h"

#include "editor/view/nodefinderstatuswidget.h"
#include "editor/view/nodefindersvgwidget.h"
#include "editor/view/nodefinderdockwidget.h"

#include "nodefindersvgconverter.h"

#include <QSvgRenderer>
#include "utils/svgutils.h"

#include <QMessageBox>

NodeFinderMgr::NodeFinderMgr(QObject *parent) :
    QObject(parent),
    drawLabels(true),
    drawStationTracks(true),
    trackPenWidth(10),
    m_isSelecting(false)
{
    converter = new NodeFinderSVGConverter(this);

    setMode(EditingModes::NoEditing);
}

EditingModes NodeFinderMgr::mode() const
{
    return m_mode;
}

void NodeFinderMgr::setMode(EditingModes m, EditingSubModes sub)
{
    m_mode = m;

    const bool isEditing = m_mode > EditingModes::NoEditing && m_mode < EditingModes::NModes;
    if(!isEditing)
        sub = EditingSubModes::NotEditingCurrentItem;
    m_subMode = sub;

    //Draw only when not editing
    drawLabels = drawStationTracks = m_subMode == EditingSubModes::NotEditingCurrentItem;

    emit modeChanged();
    emit repaintSVG();
}

EditingSubModes NodeFinderMgr::getSubMode() const
{
    return m_subMode;
}

QWidget *NodeFinderMgr::getStatusWidget(QWidget *parent)
{
    if(statusWidget && statusWidget->parent() == parent)
        return statusWidget;

    //Create a new one
    statusWidget = new NodeFinderStatusWidget(this, parent);
    return statusWidget;
}

QWidget *NodeFinderMgr::getCentralWidget(QWidget *parent)
{
    if(centralWidget && centralWidget->parent() == parent)
        return centralWidget;

    //Create a new one
    NodeFinderSVGWidget *w = new NodeFinderSVGWidget(this, parent);
    w->setRenderer(converter->renderer());
    connect(this, &NodeFinderMgr::repaintSVG, w, QOverload<>::of(&QWidget::update));

    centralWidget = w;
    return centralWidget;
}

QWidget *NodeFinderMgr::getDockWidget(QWidget *parent)
{
    if(dockWidget && dockWidget->parent() == parent)
        return dockWidget;

    //Create a new one
    NodeFinderDockWidget *w = new NodeFinderDockWidget(this, parent);
    w->setModels(converter->getLabelsModel(), converter->getTracksModel());

    dockWidget = w;
    return dockWidget;
}

bool NodeFinderMgr::loadSVG(QIODevice *dev)
{
    clearCurrentItem();

    bool ret = converter->load(dev);
    if(!ret)
    {
        converter->clear();
        setMode(EditingModes::NoSVGLoaded);
    }

    converter->processElements();
    converter->loadLabelsAndTracks();

    setTrackPenWidth(converter->calcDefaultTrackPenWidth());

    setMode(EditingModes::NoEditing);
    return true;
}

bool NodeFinderMgr::saveSVG(QIODevice *dev)
{
    return converter->save(dev);
}

void NodeFinderMgr::selectCurrentElem()
{
    if(m_subMode == EditingSubModes::AddingSubElement)
    {
        //Walk elements
        if(!converter->currentWalker.isValid())
        {
            if(centralWidget)
            {
                QMessageBox::warning(centralWidget, tr("No selection"),
                                     tr("There is no element selected.\n"
                                        "Use 'Prev' and 'Next' to select one."));
            }
            return;
        }

        if(!converter->addCurrentElementToItem())
        {
            if(centralWidget)
            {
                QMessageBox::warning(centralWidget, tr("Unsupported element"),
                                     tr("The element could not be added to current item."));
            }
        }

        requestEndEditItem();
    }
    else if(m_subMode == EditingSubModes::RemovingSubElement)
    {
        auto item = converter->getCurItem();
        if(!item)
        {
            if(centralWidget)
            {
                QMessageBox::warning(centralWidget, tr("No item"),
                                     tr("No item selected."));
            }
            return;
        }

        int subIdx = converter->getCurItemSubElemIdx();

        if(subIdx < 0 || subIdx >= item->elements.size())
        {
            if(centralWidget)
            {
                QMessageBox::warning(centralWidget, tr("No sub element"),
                                     tr("No sub element of current item was selected."));
            }
            return;
        }

        converter->removeCurrentSubElementFromItem();
    }

    emit repaintSVG();
}

void NodeFinderMgr::goToPrevElem()
{
    if(m_subMode == EditingSubModes::AddingSubElement)
    {
        //Walk elements
        while (true)
        {
            if(!converter->currentWalker.prev())
            {
                if(centralWidget)
                {
                    QMessageBox::warning(centralWidget, tr("Start"),
                                         tr("There aren't previous elements in current selection.\n"
                                            "Go forward with 'Next' to select one."));
                }
                converter->curElementPath = ElementPath(); //Reset
                break;
            }

            QDomElement elem = converter->currentWalker.element();
            QString id = elem.attribute(svg_attr::ID);
            const QRectF bounds = converter->renderer()->boundsOnElement(id);
            if(getSelectionRect().contains(bounds))
            {
                //It's in selection
                converter->curElementPath.elem = elem;
                converter->curElementPath.path.clear();
                if(utils::convertElementToPath(elem, converter->curElementPath.path))
                    break; //Can be converted to path so use it.
            }
        }
    }
    else if(m_subMode == EditingSubModes::RemovingSubElement)
    {
        auto item = converter->getCurItem();
        if(!item)
        {
            if(centralWidget)
            {
                QMessageBox::warning(centralWidget, tr("No item"),
                                     tr("No item selected."));
            }
            return;
        }

        int subIdx = converter->getCurItemSubElemIdx();
        subIdx--;
        if(subIdx < 0)
        {
            if(item->elements.isEmpty())
                subIdx = -1;
            else
                subIdx = 0;
        }
        converter->setCurItemSubElemIdx(subIdx);

        if(subIdx < 0)
        {
            if(centralWidget)
            {
                QMessageBox::warning(centralWidget, tr("No sub elements"),
                                     tr("Current item has no sub elements."));
            }
            return;
        }
    }

    emit repaintSVG();
}

void NodeFinderMgr::goToNextElem()
{
    if(m_subMode == EditingSubModes::AddingSubElement)
    {
        //Walk elements
        while (true)
        {
            if(!converter->currentWalker.next())
            {
                if(centralWidget)
                {
                    QMessageBox::warning(centralWidget, tr("End"),
                                         tr("There aren't other elements in current selection.\n"
                                            "Go back with 'Prev' to select one."));
                }
                converter->curElementPath = ElementPath(); //Reset
                break;
            }

            QDomElement elem = converter->currentWalker.element();
            QString id = elem.attribute(svg_attr::ID);
            const QRectF bounds = converter->renderer()->boundsOnElement(id);
            if(getSelectionRect().contains(bounds))
            {
                //It's in selection
                converter->curElementPath.elem = elem;
                converter->curElementPath.path.clear();
                if(utils::convertElementToPath(elem, converter->curElementPath.path))
                    break; //Can be converted to path so use it.
            }
        }
    }
    else if(m_subMode == EditingSubModes::RemovingSubElement)
    {
        auto item = converter->getCurItem();
        if(!item)
        {
            if(centralWidget)
            {
                QMessageBox::warning(centralWidget, tr("No item"),
                                     tr("No item selected."));
            }
            return;
        }

        int subIdx = converter->getCurItemSubElemIdx();
        subIdx++;
        if(subIdx >= item->elements.size())
        {
            subIdx = item->elements.size() - 1;
        }
        converter->setCurItemSubElemIdx(subIdx);

        if(subIdx < 0)
        {
            if(centralWidget)
            {
                QMessageBox::warning(centralWidget, tr("No sub elements"),
                                     tr("Current item has no sub elements."));
            }
            return;
        }
    }

    emit repaintSVG();
}

void NodeFinderMgr::requestAddSubElement()
{
    setMode(m_mode, EditingSubModes::AddingSubElement);
    clearSelection();
}

void NodeFinderMgr::requestRemoveSubElement()
{
    setMode(m_mode, EditingSubModes::RemovingSubElement);
    clearSelection();
}

void NodeFinderMgr::requestEndEditItem()
{
    setMode(m_mode, EditingSubModes::NotEditingCurrentItem);
    clearSelection();
}

void NodeFinderMgr::clearCurrentItem()
{
    //requestEndEditItem();
    converter->setCurItem(nullptr);
    clearSelection();
    setMode(EditingModes::NoEditing);
}

void NodeFinderMgr::requestEditItem(ItemBase *item, EditingModes m)
{
    clearSelection();
    converter->setCurItem(item);
    setMode(m, EditingSubModes::NotEditingCurrentItem);
}

void NodeFinderMgr::setTrackPenWidth(int value)
{
    if(trackPenWidth == value)
        return;
    trackPenWidth = value;
    emit trackPenWidthChanged(trackPenWidth);
    emit repaintSVG();
}

void NodeFinderMgr::startSelection(const QPointF &p)
{
    clearSelection();
    m_isSelecting = true;
    selectionStart = selectionEnd = p;
    emit repaintSVG();
}

void NodeFinderMgr::endOrMoveSelection(const QPointF &p, bool isEnd)
{
    if(!m_isSelecting)
        return;
    selectionEnd = p;

    if(isEnd)
    {
        m_isSelecting = false;

        if(m_subMode == EditingSubModes::AddingSubElement)
        {
            //Restart element selection
            QStringList tags{svg_tag::PathTag, svg_tag::LineTag, svg_tag::PolylineTag};
            if(m_mode == EditingModes::LabelEditing)
                tags.prepend(svg_tag::RectTag);
            converter->currentWalker = converter->walkElements(tags);
            converter->curElementPath = ElementPath(); //Reset
        }
    }

    emit repaintSVG();
}

void NodeFinderMgr::clearSelection()
{
    m_isSelecting = false;
    selectionStart = selectionEnd = QPointF();
    converter->currentWalker = NodeFinderElementWalker(); //Reset
    converter->curElementPath = ElementPath();
    emit repaintSVG();
}
