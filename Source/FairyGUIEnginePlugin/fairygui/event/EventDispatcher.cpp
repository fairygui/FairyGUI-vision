#include "EventDispatcher.h"
#include "core/DisplayObject.h"
#include "FGUIManager.h"

NS_FGUI_BEGIN

const EventTag EventTag::None;

EventTag::EventTag() :_value(0)
{
}

EventTag::EventTag(void * ptr) : _value((uintptr_t)ptr)
{
}

EventTag::EventTag(int value) : _value(value)
{
}

EventTag::EventTag(const EventTag & other)
{
    *this = other;
}

EventTag::EventTag(EventTag && other)
{
    *this = std::move(other);
}

EventTag::~EventTag()
{
}

EventTag & EventTag::operator=(const EventTag & other)
{
    if (this != &other)
        _value = other._value;
    return *this;
}

EventTag & EventTag::operator=(EventTag && other)
{
    if (this != &other)
    {
        _value = other._value;
        other._value = 0;
    }
    return *this;
}

EventTag & EventTag::operator=(void * ptr)
{
    _value = (uintptr_t)ptr;
    return *this;
}

EventTag & EventTag::operator=(int v)
{
    _value = v;
    return *this;
}

bool EventTag::operator!=(const EventTag & v)
{
    return _value != v._value;
}

bool EventTag::operator!=(const EventTag & v) const
{
    return _value != v._value;
}

bool EventTag::operator==(const EventTag & v)
{
    return _value == v._value;
}

bool EventTag::operator==(const EventTag & v) const
{
    return _value == v._value;
}

EventDispatcher::EventDispatcher() :_dispatching(0), _spectator(nullptr)
{
}

EventDispatcher::~EventDispatcher()
{
    _dispatching = 0;
    removeAllListeners();
}

void EventDispatcher::addListener(int eventType, const EventCallback& callback, const EventTag& tag, int piority)
{
    if (!tag.isNone())
    {
        for (auto it = _callbacks.begin(); it != _callbacks.end(); it++)
        {
            if ((*it)->eventType == eventType && (*it)->tag == tag)
            {
                (*it)->callback = callback;
                return;
            }
        }
    }

    EventCallbackItem* item = new EventCallbackItem();
    item->callback = callback;
    item->eventType = eventType;
    item->tag = tag;
    item->piority = piority;
    item->dispatching = 0;

    if (piority != 0)
    {
        for (auto it = _callbacks.begin(); it != _callbacks.end(); it++)
        {
            if ((*it)->eventType == eventType && (*it)->piority < piority)
            {
                _callbacks.insert(it, item);
                return;
            }
        }
    }

    _callbacks.push_back(item);
}

void EventDispatcher::removeListener(int eventType, const EventTag& tag)
{
    if (_callbacks.empty())
        return;

    for (auto it = _callbacks.begin(); it != _callbacks.end(); )
    {
        if ((*it)->eventType == eventType && ((*it)->tag == tag || tag.isNone()))
        {
            if (_dispatching > 0)
            {
                (*it)->callback = nullptr;
                it++;
            }
            else
            {
                delete (*it);
                it = _callbacks.erase(it);
            }
        }
        else
            it++;
    }
}

void EventDispatcher::removeAllListeners()
{
    if (_callbacks.empty())
        return;

    if (_dispatching > 0)
    {
        for (auto it = _callbacks.begin(); it != _callbacks.end(); ++it)
            (*it)->callback = nullptr;
    }
    else
    {
        for (auto it = _callbacks.begin(); it != _callbacks.end(); it++)
            delete (*it);
        _callbacks.clear();
    }
}

bool EventDispatcher::hasListener(int eventType, const EventTag& tag) const
{
    if (_callbacks.empty())
        return false;

    for (auto it = _callbacks.cbegin(); it != _callbacks.cend(); ++it)
    {
        if ((*it)->eventType == eventType && ((*it)->tag == tag || tag.isNone()) && (*it)->callback != nullptr)
            return true;
    }
    return false;
}

bool EventDispatcher::dispatchEvent(int eventType, void* data, const Value& dataValue)
{
    if (_callbacks.empty())
    {
        if (_spectator)
            return _spectator->dispatchEvent(eventType, data, dataValue);
        else
            return false;
    }

    EventContext context;
    context._sender = this;
    context._type = eventType;
    context._dataValue = dataValue;
    context._data = data;

    doDispatch(eventType, &context);

    return context._defaultPrevented;
}

bool EventDispatcher::isDispatchingEvent(int eventType)
{
    for (auto it = _callbacks.begin(); it != _callbacks.end(); ++it)
    {
        if ((*it)->eventType == eventType)
            return (*it)->dispatching > 0;
    }
    return false;
}

void EventDispatcher::doDispatch(int eventType, EventContext* context)
{
    retain();

    _dispatching++;
    context->_sender = this;
    bool hasDeletedItems = false;

    size_t cnt = _callbacks.size(); //dont use iterator, because new item would be added in callback.
    for (size_t i = 0; i < cnt; i++)
    {
        EventCallbackItem* ci = _callbacks[i];
        if (ci->callback == nullptr)
        {
            hasDeletedItems = true;
            continue;
        }
        if (ci->eventType == eventType)
        {
            ci->dispatching++;
            context->_touchCapture = 0;
            ci->callback(context);
            ci->dispatching--;

            if (context->_touchCapture != 0)
            {
                if (context->_touchCapture == 1 && eventType == UIEventType::TouchBegin)
                    StageInst->addTouchMonitor(context->getInput()->getTouchId(), dynamic_cast<Node*>(this));
                else if (context->_touchCapture == 2)
                    StageInst->removeTouchMonitor(dynamic_cast<Node*>(this));
            }
        }
    }

    _dispatching--;
    if (hasDeletedItems && _dispatching == 0)
    {
        for (auto it = _callbacks.begin(); it != _callbacks.end(); )
        {
            if ((*it)->callback == nullptr)
            {
                delete (*it);
                it = _callbacks.erase(it);
            }
            else
                it++;
        }
    }

    if (_spectator)
        _spectator->doDispatch(eventType, context);

    release();
}

bool EventDispatcher::bubbleEvent(int eventType, void* data, const Value& dataValue)
{
    EventContext context;
    context._type = eventType;
    context._dataValue = dataValue;
    context._data = data;

    doBubble(eventType, &context);

    return context._defaultPrevented;
}

void EventDispatcher::doBubble(int eventType, EventContext* context)
{
    //parent maybe disposed in callbacks
    WeakPtr wptr(dynamic_cast<Node*>(this)->getParentNode());

    if (!_callbacks.empty())
    {
        context->_isStopped = false;
        doDispatch(eventType, context);
        if (context->_isStopped)
            return;
    }
    else if (_spectator)
    {
        retain();
        context->_isStopped = false;
        _spectator->doDispatch(eventType, context);
        release();

        if (context->_isStopped)
            return;
    }

    Node* p = wptr.ptr();
    if (p)
        p->doBubble(eventType, context);
}

bool EventDispatcher::broadcastEvent(int eventType, void* data, const Value& dataValue)
{
    EventContext context;
    context._type = eventType;
    context._dataValue = dataValue;
    context._data = data;

    Vector<Node*> callChain;
    collectChild(dynamic_cast<DisplayObject*>(this), eventType, callChain);

    for (auto it = callChain.begin(); it != callChain.end(); it++)
    {
        (*it)->doDispatch(eventType, &context);
    }

    return context._defaultPrevented;
}

void EventDispatcher::collectChild(DisplayObject * container, int eventType, Vector<Node*>& callChain)
{
    callChain.pushBack(container);

    int cnt = container->numChildren();
    for (int i = 0; i < cnt; i++)
    {
        DisplayObject* child = container->getChildAt(i);
        if (child->numChildren() > 0)
            collectChild(child, eventType, callChain);
        else
            callChain.pushBack(child);
    }
}

NS_FGUI_END