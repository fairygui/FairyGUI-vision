#ifndef __UIEVENTDISPATCHER_H__
#define __UIEVENTDISPATCHER_H__

#include "FGUIMacros.h"
#include "EventContext.h"
#include "UIEventType.h"

NS_FGUI_BEGIN

typedef std::function<void(EventContext* context)> EventCallback;

class FGUI_IMPEXP EventTag
{
public:
    static const EventTag None;

    EventTag();
    explicit EventTag(void* ptr);
    explicit EventTag(int value);
    explicit EventTag(const EventTag& other);
    explicit EventTag(EventTag&& other);
    ~EventTag();

    EventTag& operator= (const EventTag& other);
    EventTag& operator= (EventTag&& other);
    EventTag& operator= (void* ptr);
    EventTag& operator= (int v);

    bool operator!= (const EventTag& v);
    bool operator!= (const EventTag& v) const;
    bool operator== (const EventTag& v);
    bool operator== (const EventTag& v) const;

    bool isNone() const { return _value == 0; }

private:
    uintptr_t _value;
};

class Node;

class FGUI_IMPEXP EventDispatcher : public Ref
{
public:
    EventDispatcher();
    virtual ~EventDispatcher();

    void addListener(int eventType, const EventCallback& callback) { return addListener(eventType, callback, EventTag::None); }
    void addListener(int eventType, const EventCallback& callback, const EventTag& tag, int piority = 0);
    void removeListener(int eventType) { removeListener(eventType, EventTag::None); }
    void removeListener(int eventType, const EventTag& tag);
    void removeAllListeners();
    bool hasListener(int eventType) const { return hasListener(eventType, EventTag::None); }
    bool hasListener(int eventType, const EventTag& tag) const;

    bool dispatchEvent(int eventType, void* data = nullptr, const Value& dataValue = Value::Null);
    bool broadcastEvent(int eventType, void* data = nullptr, const Value& dataValue = Value::Null);
    bool bubbleEvent(int eventType, void* data = nullptr, const Value& dataValue = Value::Null);

    bool isDispatchingEvent(int eventType);

    EventDispatcher* getSpectator() const { return _spectator; }
    void setSpectator(EventDispatcher* value) { _spectator = value; }

private:
    void doDispatch(int eventType, EventContext* context);
    void doBubble(int eventType, EventContext* context);

    void collectChild(DisplayObject* container, int eventType, Vector<Node*>& callChain);

    struct EventCallbackItem
    {
        EventCallback callback;
        int eventType;
        EventTag tag;
        int piority;
        int dispatching;
    };
    std::vector<EventCallbackItem*> _callbacks;
    int _dispatching;
    EventDispatcher* _spectator;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(EventDispatcher);
};

NS_FGUI_END

#endif
