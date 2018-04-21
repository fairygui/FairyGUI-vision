#ifndef __NODE_H__
#define __NODE_H__

#include "FGUIMacros.h"
#include "event/EventDispatcher.h"

#include "third_party/cc/CCScheduler.h"
#include "third_party/cc/CCActionManager.h"
#include "third_party/cc/CCActionInterval.h"
#include "third_party/cc/CCActionInstant.h"
#include "third_party/cc/CCActionEase.h"

NS_FGUI_BEGIN

class WeakPtr;

class FGUI_IMPEXP Node : public EventDispatcher
{
public:
    virtual Node* getParentNode() const { return nullptr;  }
    virtual bool onStage() const { return false; }

    Scheduler *  getScheduler() const { return _scheduler; }
    ActionManager* getActionManager() const { return _actionManager; }

    void runAction(Action* action);
    void stopAction(Action* action);
    void stopActionByTag(int tag);
    void stopAllActionsByTag(int tag);
    Action* getActionByTag(int tag);

    void schedule(SEL_SCHEDULE selector, float interval, unsigned int repeat, float delay);
    void schedule(SEL_SCHEDULE selector, float interval);
    void scheduleOnce(SEL_SCHEDULE selector, float delay = 0) { schedule(selector, 0, 0, delay); }
    void scheduleOnce(const ccSchedulerFunc& callback, float delay, const std::string& key);
    void unSchedule(SEL_SCHEDULE selector);
    void unSchedule(const std::string& key);

protected:
    Node();
    virtual ~Node();

protected:
    INT64 _intID;

private:
    size_t _weakPtrRef;
    Scheduler* _scheduler;
    ActionManager* _actionManager;

    friend class WeakPtr;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(Node);
};

class FGUI_IMPEXP WeakPtr
{
public:
    WeakPtr();
    explicit WeakPtr(Node* obj);
    explicit WeakPtr(const WeakPtr& other);
    explicit WeakPtr(WeakPtr&& other);
    ~WeakPtr();

    WeakPtr& operator= (const WeakPtr& other);
    WeakPtr& operator= (WeakPtr&& other);
    WeakPtr& operator= (Node* obj);
    bool operator!= (const WeakPtr& v);
    bool operator!= (const WeakPtr& v) const;
    bool operator== (const WeakPtr& v);
    bool operator== (const WeakPtr& v) const;
    bool operator== (const Node* v);
    bool operator== (const Node* v) const { return ptr() == v; }

    Node* ptr() const;
    template<typename T> T* ptr() const { return dynamic_cast<T*>(ptr()); }
    bool onStage() const;

private:
    INT64 _intID;

    static INT64 add(Node * obj);
    static Node* remove(INT64 id);
    static void markDisposed(Node* obj);

    friend class Node;
};

NS_FGUI_END

#endif