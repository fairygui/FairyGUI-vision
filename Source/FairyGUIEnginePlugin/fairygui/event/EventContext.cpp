#include "EventContext.h"
#include "FGUIManager.h"

NS_FGUI_BEGIN

EventContext::EventContext() :
    _sender(nullptr),
    _data(nullptr),
    _isStopped(false),
    _defaultPrevented(false),
    _touchCapture(0),
    _type(0)
{
}

EventContext::~EventContext()
{

}

InputEvent * EventContext::getInput() const
{
    return StageInst->getRecentInput();
}


NS_FGUI_END