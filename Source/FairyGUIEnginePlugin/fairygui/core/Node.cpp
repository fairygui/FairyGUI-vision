#include "Node.h"
#include "FGUIManager.h"

NS_FGUI_BEGIN

Node::Node()
{
    static INT64 _gInstanceCounter = 1;
    _intID = _gInstanceCounter++;
    _weakPtrRef = 0;

    _actionManager = FGUIManager::GlobalManager().getActionManager();
    _actionManager->retain();

    _scheduler = FGUIManager::GlobalManager().getScheduler();
    _scheduler->retain();
}

Node::~Node()
{
    if (_weakPtrRef > 0)
        WeakPtr::markDisposed(this);

    _actionManager->removeAllActionsFromTarget(this);
    _scheduler->unscheduleAllForTarget(this);

    _actionManager->release();
    _scheduler->release();
}

void Node::runAction(Action * action)
{
    _actionManager->addAction(action, this, false);
}

void Node::stopAction(Action* action)
{
    _actionManager->removeAction(action);
}

void Node::stopActionByTag(int tag)
{
    CCASSERT(tag != Action::INVALID_TAG, "Invalid tag");
    _actionManager->removeActionByTag(tag, this);
}

void Node::stopAllActionsByTag(int tag)
{
    CCASSERT(tag != Action::INVALID_TAG, "Invalid tag");
    _actionManager->removeAllActionsByTag(tag, this);
}

Action * Node::getActionByTag(int tag)
{
    return _actionManager->getActionByTag(tag, this);
}

void Node::schedule(SEL_SCHEDULE selector, float interval, unsigned int repeat, float delay)
{
    _scheduler->schedule(selector, this, interval, repeat, delay, false);
}

void Node::schedule(SEL_SCHEDULE selector, float interval)
{
    _scheduler->schedule(selector, this, interval, false);
}

void Node::scheduleOnce(const ccSchedulerFunc & callback, float delay, const std::string & key)
{
    _scheduler->schedule(callback, this, 0, 0, delay, false, key);
}

void Node::unSchedule(SEL_SCHEDULE selector)
{
    _scheduler->unschedule(selector, this);
}

void Node::unSchedule(const std::string & key)
{
    _scheduler->unschedule(key, this);
}

static std::unordered_map<INT64, Node*> _weakPointers;

WeakPtr::WeakPtr() :_intID(0)
{
}

WeakPtr::WeakPtr(Node * obj)
{
    _intID = add(obj);
}

WeakPtr::WeakPtr(const WeakPtr & other) :_intID(0)
{
    *this = other;
}

WeakPtr::WeakPtr(WeakPtr && other) : _intID(0)
{
    *this = std::move(other);
}

WeakPtr::~WeakPtr()
{
    if (_intID != 0)
        remove(_intID);
}

WeakPtr & WeakPtr::operator=(const WeakPtr & other)
{
    if (_intID != 0)
        remove(_intID);
    _intID = add(other.ptr());
    return *this;
}

WeakPtr & WeakPtr::operator=(WeakPtr && other)
{
    if (this != &other)
    {
        if (_intID != 0)
            remove(_intID);
        _intID = other._intID;
        other._intID = 0;
    }

    return *this;
}

WeakPtr & WeakPtr::operator=(Node * obj)
{
    if (_intID != 0)
        remove(_intID);
    _intID = add(obj);
    return *this;
}

bool WeakPtr::operator!=(const WeakPtr & v)
{
    return !(*this == v);
}

bool WeakPtr::operator!=(const WeakPtr & v) const
{
    return !(*this == v);
}

bool WeakPtr::operator==(const WeakPtr & v)
{
    const auto &t = *this;
    return t == v;
}

bool WeakPtr::operator==(const WeakPtr & v) const
{
    return _intID == v._intID;
}

bool WeakPtr::operator==(const Node * v)
{
    const auto &t = *this;
    return t == v;
}

Node * WeakPtr::ptr() const
{
    if (_intID == 0)
        return nullptr;

    auto it = _weakPointers.find(_intID);
    if (it != _weakPointers.end())
        return it->second;
    else
        return nullptr;
}

bool WeakPtr::onStage() const
{
    Node *p = ptr();
    return p  && p->onStage();
}

INT64 WeakPtr::add(Node * obj)
{
    if (obj)
    {
        if (obj->_weakPtrRef == 0)
            _weakPointers[obj->_intID] = obj;
        obj->_weakPtrRef++;
        return obj->_intID;
    }
    else
        return 0;
}

Node* WeakPtr::remove(INT64 id)
{
    auto it = _weakPointers.find(id);
    if (it != _weakPointers.end())
    {
        Node* obj = it->second;
        obj->_weakPtrRef--;
        if (obj->_weakPtrRef == 0)
            _weakPointers.erase(it);
        return obj;
    }
    else
        return nullptr;
}

void WeakPtr::markDisposed(Node * obj)
{
    auto it = _weakPointers.find(obj->_intID);
    if (it != _weakPointers.end())
        _weakPointers.erase(it);
}


NS_FGUI_END