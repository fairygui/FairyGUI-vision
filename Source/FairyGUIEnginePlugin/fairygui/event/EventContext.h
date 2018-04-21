#ifndef __EVENTCONTEXT_H__
#define __EVENTCONTEXT_H__

#include "FGUIMacros.h"
#include "InputEvent.h"

NS_FGUI_BEGIN

class EventDispatcher;

class FGUI_IMPEXP EventContext
{
public:
    EventContext();
    ~EventContext();

    int getType() const { return _type; }
    EventDispatcher* getSender() const { return _sender; }
    InputEvent* getInput() const;
    void stopPropagation() { _isStopped = true; }
    void preventDefault() { _defaultPrevented = true; }
    bool isDefaultPrevented() { return _defaultPrevented; }
    void captureTouch() { _touchCapture = 1; }
    void uncaptureTouch() { _touchCapture = 2; }

    const Value& getDataValue() const { return _dataValue; }
    void* getData() const { return _data; }

private:
    EventDispatcher* _sender;
    Value _dataValue;
    void* _data;
    bool _isStopped;
    bool _defaultPrevented;
    int _touchCapture;
    int _type;

    friend class EventDispatcher;
};

NS_FGUI_END

#endif
