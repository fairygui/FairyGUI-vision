#ifndef __INPUTEVENT_H__
#define __INPUTEVENT_H__

#include "FGUIMacros.h"

NS_FGUI_BEGIN

class DisplayObject;
class Stage;

class FGUI_IMPEXP InputEvent
{
public:
    InputEvent();
    ~InputEvent();

    DisplayObject* getTarget() const { return _target; }
    int getX() const { return (int)_pos.x; }
    int getY() const { return (int)_pos.y; }
    const hkvVec2& getPosition() const { return _pos; }
    int getTouchId()const { return _touchId; }
    int isDoubleClick()const { return _clickCount == 2; }
    int getButton() const { return _button; }
    int getKeyCode() const { return _keyCode; }
    const std::string getKeyName() const { return _keyName; }
    bool isCtrlDown() const { return (_keyModifiers & 1)!=0; }
    bool isAltDown() const  { return (_keyModifiers & 2) != 0; }
    bool isShiftDown() const { return (_keyModifiers & 4) != 0; }
    int getMouseWheelDelta() const { return _mouseWheelDelta; }

private:
    DisplayObject* _target;
    hkvVec2 _pos;
    int _touchId;
    int _clickCount;
    int _mouseWheelDelta;
    int _button;
    int _keyCode;
    int _keyModifiers;
    std::string _keyName;

    friend class Stage;
};

NS_FGUI_END

#endif
