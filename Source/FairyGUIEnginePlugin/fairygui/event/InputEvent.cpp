#include "InputEvent.h"

NS_FGUI_BEGIN

InputEvent::InputEvent() :
    _target(nullptr),
    _touchId(-1),
    _clickCount(0),
    _mouseWheelDelta(0),
    _button(-1),
    _keyCode(0),
    _keyModifiers(0),
    _pos(0,0)
{
}

InputEvent::~InputEvent()
{

}


NS_FGUI_END