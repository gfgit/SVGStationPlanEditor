#include "nodefindermgr.h"

#include "view/nodefinderstatuswidget.h"
#include "view/nodefindersvgwidget.h"
#include "view/nodefinderdockwidget.h"

#include "nodefindersvgconverter.h"

#include <QSvgRenderer>
#include "ssplib/utils/svg_path_utils.h"
#include "ssplib/utils/svg_constants.h"

#include "elementsplitterhelper.h"

#include <QMessageBox>

NodeFinderMgr::NodeFinderMgr(QObject *parent) :
    QObject(parent),
    m_isSelecting(false),
    m_isSinglePoint(false)
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

    if(sub == EditingSubModes::DoSplitItem && m_mode != EditingModes::SplitElement)
        sub = EditingSubModes::NotEditingCurrentItem;

    if(m_mode == EditingModes::SplitElement && sub != EditingSubModes::DoSplitItem)
    {
        sub = EditingSubModes::AddingSubElement;
    }

    m_subMode = sub;

    //Draw only when not editing
    auto plan = getStationPlan();
    plan->drawLabels = plan->drawTracks = m_subMode == EditingSubModes::NotEditingCurrentItem;

    emit modeChanged();
    emit repaintSVG();
}

bool NodeFinderMgr::validateCurrentElement()
{
    constexpr const qreal MinStrokeWidth = 5;

    ssplib::ElementPath elemPath;
    elemPath.elem = converter->currentWalker.element();
    if(!ssplib::utils::convertElementToPath(elemPath.elem, elemPath.path))
        return false; //Canmot be converted to path, skip it.

    elemPath.strokeWidth = 0;
    QRectF bounds = elemPath.path.boundingRect();
    if(!ssplib::utils::parseStrokeWidth(elemPath.elem, bounds, elemPath.strokeWidth))
        elemPath.strokeWidth = 0;

    //Null rect breaks QRectF::contains() which returns always false
    if(bounds.width() == 0)
        bounds.setWidth(1);
    if(bounds.height() == 0)
        bounds.setHeight(1);

    bool isElementValid = false;

    if(m_isSinglePoint)
    {
        //Single point, element contains point
        double width = elemPath.strokeWidth;
        if(width < MinStrokeWidth)
            width = MinStrokeWidth;

        const QPointF offset(width, width);
        QRectF targetRect(selectionStart - offset, selectionStart + offset);

        isElementValid = elemPath.path.intersects(targetRect);
    }
    else
    {
        //Rect, selection contains element
        const QRectF selection = getSelectionRect();
        isElementValid = selection.contains(bounds);
    }

    if(!isElementValid)
        return false; //Go to next element

    converter->curElementPath = elemPath;
    return true; //Use element
}

EditingSubModes NodeFinderMgr::getSubMode() const
{
    return m_subMode;
}

QString NodeFinderMgr::getModeName(EditingModes mode) const
{
    QString modeName;
    switch (mode)
    {
    case EditingModes::NoSVGLoaded:
        modeName = tr("No SVG");
        break;
    case EditingModes::NoEditing:
        modeName = tr("No Editing");
        break;
    case EditingModes::LabelEditing:
        modeName = tr("Label Editing");
        break;
    case EditingModes::StationTrackEditing:
        modeName = tr("Station Track Editing");
        break;
    case EditingModes::TrackPathEditing:
        modeName = tr("Track Path Editing");
        break;
    case EditingModes::SplitElement:
        modeName = tr("Split Element");
        break;
    case EditingModes::NModes:
        modeName = tr("Unknown mode");
    }

    return modeName;
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
    NodeFinderSVGWidget *w = new NodeFinderSVGWidget(nullptr, this, parent);
    w->setRenderer(converter->renderer());
    connect(this, &NodeFinderMgr::repaintSVG, w, QOverload<>::of(&QWidget::update));

    centralWidget = w;
    return centralWidget;
}

QWidget *NodeFinderMgr::getDockWidget(EditingModes mode)
{
    //Create a new one
    NodeFinderDockWidget *w = new NodeFinderDockWidget(this);
    w->setModel(converter->getModel(mode), getModeName(mode));

    return w;
}

int NodeFinderMgr::getTrackPenWidth() const
{
    return getStationPlan()->platformPenWidth;
}

bool NodeFinderMgr::loadSVG(QIODevice *dev)
{
    clearCurrentItem();

    bool ret = converter->loadDocument(dev);
    if(!ret)
    {
        converter->clear();
        setMode(EditingModes::NoSVGLoaded);
    }

    //Convert SVG
    converter->processElements();

    //Load view
    converter->reloadSVGRenderer();

    setTrackPenWidth(converter->calcDefaultTrackPenWidth());

    setMode(EditingModes::NoEditing);
    return true;
}

bool NodeFinderMgr::saveSVG(QIODevice *dev)
{
    return converter->save(dev);
}

ssplib::StationPlan *NodeFinderMgr::getStationPlan() const
{
    return &converter->m_plan;
}

ssplib::EditingInfo *NodeFinderMgr::getEditingInfo() const
{
    return &converter->m_info;
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

        if(m_mode == EditingModes::SplitElement)
        {
            setMode(m_mode, EditingSubModes::DoSplitItem);
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

        if(!m_isSinglePoint)
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
            bool wasValid = converter->currentWalker.isValid();
            const NodeFinderElementWalker::Status status = converter->currentWalker.getStatus();
            if(!converter->currentWalker.prev())
            {
                if(centralWidget)
                {
                    QMessageBox::warning(centralWidget, tr("Start"),
                                         tr("There aren't previous elements in current selection.\n"
                                            "Go forward with 'Next' to select one."));
                }

                if(m_isSinglePoint && wasValid)
                {
                    //Keep last item selected
                    converter->currentWalker.restoreStatus(status);
                }
                else
                {
                    converter->curElementPath = ssplib::ElementPath(); //Reset
                }
                break;
            }

            if(validateCurrentElement())
                break; //Use element
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
            bool wasValid = converter->currentWalker.isValid();
            const NodeFinderElementWalker::Status status = converter->currentWalker.getStatus();
            if(!converter->currentWalker.next())
            {
                if(centralWidget)
                {
                    QMessageBox::warning(centralWidget, tr("End"),
                                         tr("There aren't other elements in current selection.\n"
                                            "Go back with 'Prev' to select one."));
                }

                if(m_isSinglePoint && wasValid)
                {
                    //Keep last item selected
                    converter->currentWalker.restoreStatus(status);
                }
                else
                {
                    converter->curElementPath = ssplib::ElementPath(); //Reset
                }
                break;
            }

            if(validateCurrentElement())
                break; //Use element
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
    EditingModes m = m_mode;
    if(m == EditingModes::SplitElement)
        m = EditingModes::NoEditing; //Close element splitting

    setMode(m, EditingSubModes::NotEditingCurrentItem);
    clearSelection();
}

void NodeFinderMgr::clearCurrentItem()
{
    //requestEndEditItem();
    converter->setCurItem(nullptr);
    clearSelection();
    setMode(EditingModes::NoEditing);
}

void NodeFinderMgr::requestEditItem(ssplib::ItemBase *item, EditingModes m)
{
    clearSelection();
    converter->setCurItem(item);
    setMode(m, EditingSubModes::NotEditingCurrentItem);
}

void NodeFinderMgr::setTrackPenWidth(int value)
{
    auto plan = getStationPlan();
    if(plan->platformPenWidth == value)
        return;
    plan->platformPenWidth = value;
    emit trackPenWidthChanged(plan->platformPenWidth);
    emit repaintSVG();
}

void NodeFinderMgr::startSelection(const QPointF &p)
{
    if(m_subMode == EditingSubModes::DoSplitItem)
    {
        //Use as split
        triggerElementSplit(p);
        return;
    }

    clearSelection();
    m_isSelecting = true;
    m_isSinglePoint = true;
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
            QStringList tags{ssplib::svg_tags::PathTag, ssplib::svg_tags::LineTag, ssplib::svg_tags::PolylineTag};
            if(m_mode == EditingModes::LabelEditing)
                tags.prepend(ssplib::svg_tags::RectTag);
            converter->currentWalker = converter->walkElements(tags);
            converter->curElementPath = ssplib::ElementPath(); //Reset

            if(m_isSinglePoint)
            {
                //Try to select first available element
                goToNextElem();
            }
        }
    }else{
        //Mouse has moved so it's not a single point
        m_isSinglePoint = false;
    }

    emit repaintSVG();
}

void NodeFinderMgr::clearSelection()
{
    m_isSelecting = false;
    m_isSinglePoint = false;
    selectionStart = selectionEnd = QPointF();
    converter->currentWalker = NodeFinderElementWalker(); //Reset
    converter->curElementPath = ssplib::ElementPath();
    emit repaintSVG();
}

void NodeFinderMgr::startElementSplitProcess()
{
    clearCurrentItem();
    setMode(EditingModes::SplitElement, EditingSubModes::AddingSubElement);
}

void NodeFinderMgr::triggerElementSplit(const QPointF& pos)
{
    int ret = QMessageBox::question(centralWidget, tr("Split Item?"),
                                    tr("Split current element at clicked point X coordinate?"));
    if(ret != QMessageBox::Yes)
        return; //Abort

    const auto plan = getStationPlan();
    ElementSplitterHelper helper(this, converter->currentWalker.element(), plan->platformPenWidth);
    if(!helper.splitAt(pos))
    {
        QMessageBox::warning(centralWidget, tr("No split"),
                             tr("Cannot calculate split point, did you click outside of path bounds?"
                                "By adjusting track pen width you can make bouns larger."));
        return;
    }

    //Current element might be removed so reset walker
    clearCurrentItem();
}
