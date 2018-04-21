#include "DragDropManager.h"
#include "UIObjectFactory.h"
#include "GRoot.h"
#include "GLoader.h"

NS_FGUI_BEGIN

DragDropManager::DragDropManager(GRoot* root) :
    _groot(root),
    _agent(nullptr)
{
    _agent = (GLoader*)UIObjectFactory::newObject("loader");
    _agent->retain();
    _agent->setTouchable(false);
    _agent->setDraggable(true);
    _agent->setSize(100, 100);
    _agent->setPivot(0.5f, 0.5f, true);
    _agent->setAlign(AlignType::CENTER);
    _agent->setVerticalAlign(VertAlignType::MIDDLE);
    _agent->setSortingOrder(INT_MAX);
    _agent->addListener(UIEventType::DragEnd, CALLBACK_1(DragDropManager::onDragEnd, this));
}

DragDropManager::~DragDropManager()
{
    CC_SAFE_RELEASE(_agent);
}

bool DragDropManager::isDragging() const
{
    return _agent->getParent() != nullptr;
}

void DragDropManager::startDrag(const std::string & icon, const Value& sourceData, int touchPointID)
{
    if (_agent->getParent() != nullptr)
        return;

    _sourceData = sourceData;
    _agent->setURL(icon);
    _groot->addChild(_agent);
    hkvVec2 pt = _groot->globalToLocal(StageInst->getTouchPosition(touchPointID));
    _agent->setPosition(pt.x, pt.y);
    _agent->startDrag(touchPointID);
}

void DragDropManager::cancel()
{
    if (_agent->getParent() != nullptr)
    {
        _agent->stopDrag();
        _groot->removeChild(_agent);
        _sourceData = Value::Null;
    }
}

void DragDropManager::onDragEnd(EventContext * context)
{
    if (_agent->getParent() == nullptr) //cancelled
        return;

    _groot->removeChild(_agent);

    GObject* obj = _groot->getTouchTarget();
    while (obj != nullptr)
    {
        if (dynamic_cast<GComponent*>(obj))
        {
            if (obj->hasListener(UIEventType::Drop))
            {
                //obj->requestFocus();
                obj->dispatchEvent(UIEventType::Drop, nullptr, _sourceData);
                return;
            }
        }

        obj = obj->getParent();
    }
}

NS_FGUI_END