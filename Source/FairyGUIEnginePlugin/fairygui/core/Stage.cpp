#include "Stage.h"
#include "RenderContext.h"
#include "Shape.h"
#include "SelectionShape.h"
#include "InputTextField.h"
#include "UIConfig.h"
#include "GRoot.h"
#include "UIPackage.h"
#include "FGUIManager.h"

NS_FGUI_BEGIN

class TouchInfo
{
public:
    TouchInfo();
    ~TouchInfo();

    void reset();

    hkvVec2 pos;
    INT_PTR touchId;
    int clickCount;
    int mouseWheelDelta;
    int button;
    hkvVec2 downPos;
    bool began;
    bool clickCancelled;
    bool moved;
    clock_t lastClickTime;
    WeakPtr target;
    WeakPtr lastRollOver;
    std::vector<WeakPtr> downTargets;
    std::vector<WeakPtr> touchMonitors;

    InputEvent evt;
};

Stage::Stage() :
    _touchCount(0),
    _touchPosition(0, 0),
    _caret(nullptr),
    _selectionShape(nullptr),
    _multiTouchInput(nullptr),
    _mouseInput(nullptr),
    _capsLockOn(false),
    _soundEnabled(true),
    _soundVolumeScale(1.0f)
{
    for (int i = 0; i < 5; i++)
        _touches.push_back(new TouchInfo());
    _recentInput = &_touches[0]->evt;

    _keyStatus = new float[256];
    for (int i = 0; i < 256; i++)
        _keyStatus[i] = -1;

    _mouseInput = static_cast<VMousePC*>(&VInputManager::GetMouse());
    if (_mouseInput == nullptr)
        _multiTouchInput = static_cast<IVMultiTouchInput*>(&VInputManager::GetInputDevice(INPUT_DEVICE_TOUCHSCREEN));

    setSize((float)Vision::Video.GetXRes(), (float)Vision::Video.GetYRes());

    _caret = Shape::create();
    _caret->setTouchable(false);
    _caret->getGraphics()->setIgnoreClipping(true);
    _caret->retain();

    _selectionShape = SelectionShape::create();
    _selectionShape->setTouchable(false);
    _selectionShape->setColor(UIConfig::inputHighlightColor);
    _selectionShape->retain();
}

Stage::~Stage()
{
    for (auto &ti : _touches)
        delete ti;
    delete _keyStatus;

    CC_SAFE_RELEASE(_caret);
    CC_SAFE_RELEASE(_selectionShape);
}

const hkvVec2& Stage::getTouchPosition()
{
    return _touchPosition;
}

const hkvVec2& Stage::getTouchPosition(int touchId)
{
    for (auto &ti : _touches)
    {
        if (ti->touchId == touchId)
            return ti->pos;
    }
    return _touchPosition;
}

DisplayObject * Stage::getTouchTarget()
{
    return _touchTarget.ptr<DisplayObject>();
}

void Stage::setFocus(DisplayObject* value)
{
    if (value == this)
        value = nullptr;

    if (_focused == value)
        return;

    DisplayObject* oldFocus = _focused.ptr<DisplayObject>();
    _focused = value;

    if (oldFocus != nullptr)
    {
        if (dynamic_cast<InputTextField*>(oldFocus))
            dynamic_cast<InputTextField*>(oldFocus)->dispatchEvent(UIEventType::FocusOut);
    }

    if (value != nullptr)
    {
        if (dynamic_cast<InputTextField*>(value))
            dynamic_cast<InputTextField*>(value)->dispatchEvent(UIEventType::FocusIn);
    }
}

void Stage::addTouchMonitor(int touchId, Node * target)
{
    TouchInfo* ti = getTouch(touchId, false);
    if (!ti)
        return;

    auto it = std::find(ti->touchMonitors.cbegin(), ti->touchMonitors.cend(), target);
    if (it == ti->touchMonitors.cend())
        ti->touchMonitors.push_back(WeakPtr(target));
}

void Stage::removeTouchMonitor(Node * target)
{
    for (auto it = _touches.cbegin(); it != _touches.cend(); ++it)
    {
        auto it2 = std::find((*it)->touchMonitors.begin(), (*it)->touchMonitors.end(), target);
        if (it2 != (*it)->touchMonitors.end())
            *it2 = nullptr;
    }
}

void Stage::cancelClick(int touchId)
{
    TouchInfo* ti = getTouch(touchId, false);
    if (ti)
        ti->clickCancelled = true;
}

void Stage::playSound(const std::string & url, float volumnScale)
{
    if (!_soundEnabled)
        return;

    PackageItem* pi = UIPackage::getItemByURL(url);
    if (pi && pi->sound)
    {
        pi->sound->SetVolume(_soundVolumeScale * volumnScale);
        pi->sound->Play();
    }
}

void Stage::setSoundEnabled(bool value)
{
    _soundEnabled = value;
}

void Stage::setSoundVolumeScale(float value)
{
    _soundVolumeScale = value;
}

void Stage::update(float dt)
{
    parseHit();

    if (_multiTouchInput != nullptr)
        handleTouchInput();
    else
        handleMouseInput();
#ifdef SUPPORTS_KEYBOARD
    handleKeyboardInput();
#endif

    DisplayObject::update(dt);
}

void Stage::parseHit()
{
    if (_multiTouchInput != nullptr)
    {
        _touchTarget = nullptr;
        for (int i = 0; i < _multiTouchInput->GetNumberOfTouchPoints(); ++i)
        {
            if (!_multiTouchInput->IsActiveTouch(i))
                continue;

            auto uTouch = _multiTouchInput->GetTouch(i);

            TouchInfo* touch = nullptr;
            TouchInfo* free = nullptr;
            for (int j = 0; j < 5; j++)
            {
                if (_touches[j]->touchId == uTouch.iID)
                {
                    touch = _touches[j];
                    break;
                }

                if (_touches[j]->touchId == -1)
                    free = _touches[j];
            }
            if (touch == nullptr)
            {
                touch = free;
                if (touch == nullptr)
                    continue;

                touch->touchId = uTouch.iID;
            }

            _touchTarget = hitTest(touch->pos, true);
            touch->target = _touchTarget;
        }
    }
    else
    {
        TouchInfo* touch = _touches[0];
        _touchPosition = hkvVec2(_mouseInput->GetRawControlValue(CT_MOUSE_ABS_X), _mouseInput->GetRawControlValue(CT_MOUSE_ABS_Y));

        touch->moved = _touchPosition != touch->pos;
        touch->pos = _touchPosition;
        touch->touchId = 0;
        _touchTarget = hitTest(_touchPosition, true);
        if (_touchTarget == nullptr)
            _touchTarget = this;
        touch->target = _touchTarget;
    }
}

void Stage::handleMouseInput()
{
    if (!FGUIManager::GlobalManager().isShowCursor())
        return;

    TouchInfo* touch = _touches[0];
    DisplayObject* touchTarget = _touchTarget.ptr<DisplayObject>();

    if (touch->moved)
        setMove(touch);

    if (touch->lastRollOver != touch->target)
        handleRollOver(touch);

    bool leftDown = _mouseInput->GetRawControlValue(CT_MOUSE_LEFT_BUTTON) != 0;
    bool rightDown = _mouseInput->GetRawControlValue(CT_MOUSE_RIGHT_BUTTON) != 0;
    bool middleDown = _mouseInput->GetRawControlValue(CT_MOUSE_MIDDLE_BUTTON) != 0;

    int wheelDelta = _mouseInput->GetRawControlValue(CT_MOUSE_WHEEL);
    if (wheelDelta != 0)
    {
        touch->mouseWheelDelta = -wheelDelta;
        updateEvent(touch);
        touchTarget->bubbleEvent(UIEventType::MouseWheel);
        touch->mouseWheelDelta = 0;
    }

    if (leftDown || rightDown || middleDown)
    {
        if (!touch->began)
        {
            _touchCount = 1;
            setBegin(touch);
            touch->button = middleDown ? 2 : (rightDown ? 1 : 0);
            setFocus(touchTarget);

            updateEvent(touch);
            dispatchEvent(UIEventType::StageTouchBegin);
            touchTarget->bubbleEvent(UIEventType::TouchBegin);
        }
    }
    else if (touch->began)
    {
        _touchCount = 0;
        setEnd(touch);

        DisplayObject* clickTarget = clickTest(touch);
        if (clickTarget != nullptr)
        {
            updateEvent(touch);

            if (touch->button == 1)
                clickTarget->bubbleEvent(UIEventType::RightClick);
            else if (touch->button == 2)
                clickTarget->bubbleEvent(UIEventType::MiddleClick);
            else
                clickTarget->bubbleEvent(UIEventType::Click);
        }

        touch->button = -1;
    }
}

void Stage::handleTouchInput()
{
    for (int i = 0; i < _multiTouchInput->GetNumberOfTouchPoints(); ++i)
    {
        if (!_multiTouchInput->IsActiveTouch(i))
            continue;

        auto uTouch = _multiTouchInput->GetTouch(i);
    }
}

void Stage::handleKeyboardInput()
{
    int modifiers = 0;

    if (VGLIsKeyPressed(VGLK_LSHIFT)) modifiers |= 4;
    if (VGLIsKeyPressed(VGLK_RSHIFT)) modifiers |= 4;
    if (VGLIsKeyPressed(VGLK_LCTRL)) modifiers |= 1;
    if (VGLIsKeyPressed(VGLK_RCTRL)) modifiers |= 1;
    if (VGLIsKeyPressed(VGLK_LALT)) modifiers |= 2;
    if (VGLIsKeyPressed(VGLK_RALT)) modifiers |= 2;

    for (int i = 1; i < 255; i++)
    {
        if (VGLIsKeyPressed(i))
            onKeyDown(i, modifiers);
        else
            _keyStatus[i] = -1;
    }
}

void Stage::onKeyDown(int keyId, int modifiers)
{
    float currTime = Vision::GetTimer()->GetTime();
    float status = _keyStatus[keyId];
    if (status < 0)
    {
        _keyStatus[keyId] = currTime + 0.5f;
    }
    else
    {
        if (currTime - status < 0)
            return;

        _keyStatus[keyId] = currTime + 0.05f;
    }

    if (keyId == VGLK_CAPS)
    {
        _capsLockOn = !_capsLockOn;
        return;
    }

    _recentInput->_keyCode = keyId;
    if (keyId >= 32 && keyId <= 126)
    {
        const VGLKey_t *pKeyTable = VGLGetKeyCharMap();
        bool bCapital = (modifiers & 4) > 0;
        if (_capsLockOn)
            bCapital = !bCapital;
        _recentInput->_keyName = static_cast<wchar_t>(bCapital ? pKeyTable[keyId].m_chUpper : pKeyTable[keyId].m_chLower);
    }
    else
        _recentInput->_keyName.clear();
    _recentInput->_keyModifiers = modifiers;
    DisplayObject* target = _focused.ptr<DisplayObject>();
    _recentInput->_target = target;

    if (target != nullptr)
        target->dispatchEvent(UIEventType::KeyDown);
    else
        dispatchEvent(UIEventType::KeyDown);
}

TouchInfo* Stage::getTouch(int touchId, bool createIfNotExisits)
{
    TouchInfo* ret = nullptr;
    for (auto &ti : _touches)
    {
        if (ti->touchId == touchId)
            return ti;
        else if (ti->touchId == -1)
            ret = ti;
    }

    if (!ret)
    {
        if (!createIfNotExisits)
            return nullptr;

        ret = new TouchInfo();
        _touches.push_back(ret);
    }
    ret->touchId = touchId;
    return ret;
}

void Stage::updateEvent(TouchInfo* ti)
{
    ti->evt._pos.x = hkvMath::floor(ti->pos.x);
    ti->evt._pos.y = hkvMath::floor(ti->pos.y);
    ti->evt._target = ti->target.ptr<DisplayObject>();
    ti->evt._clickCount = ti->clickCount;
    ti->evt._button = ti->button;
    ti->evt._mouseWheelDelta = ti->mouseWheelDelta;
    ti->evt._touchId = ti->touchId;
    _recentInput = &ti->evt;
}

void Stage::handleRollOver(TouchInfo* touch)
{
    DisplayObject* target = touch->target.ptr<DisplayObject>();
    DisplayObject* element = touch->lastRollOver.ptr<DisplayObject>();
    if (target == element)
        return;

    std::vector<WeakPtr> rollOutChain;
    std::vector<WeakPtr> rollOverChain;

    while (element != nullptr)
    {
        rollOutChain.push_back(WeakPtr(element));
        element = element->getParent();
    }

    element = target;
    while (element != nullptr)
    {
        auto iter = std::find(rollOutChain.cbegin(), rollOutChain.cend(), element);
        if (iter != rollOutChain.cend())
        {
            rollOutChain.resize(iter - rollOutChain.cbegin());
            break;
        }
        rollOverChain.push_back(WeakPtr(element));

        element = element->getParent();
    }

    touch->lastRollOver = target;

    for (auto &wptr : rollOutChain)
    {
        element = wptr.ptr<DisplayObject>();
        if (element && element->onStage())
            element->dispatchEvent(UIEventType::RollOut);
    }

    for (auto &wptr : rollOverChain)
    {
        element = wptr.ptr<DisplayObject>();
        if (element && element->onStage())
            element->dispatchEvent(UIEventType::RollOver);
    }
}

void Stage::setBegin(TouchInfo* touch)
{
    touch->began = true;
    touch->clickCancelled = false;
    touch->downPos = touch->pos;

    touch->downTargets.clear();
    DisplayObject* obj = touch->target.ptr<DisplayObject>();
    while (obj != nullptr)
    {
        touch->downTargets.push_back(WeakPtr(obj));
        obj = obj->getParent();
    }
}

void Stage::setEnd(TouchInfo* touch)
{
    touch->began = false;

    updateEvent(touch);

    DisplayObject* target = touch->target.ptr<DisplayObject>();
    if (touch->touchMonitors.size() > 0)
    {
        int len = touch->touchMonitors.size();
        for (int i = 0; i < len; i++)
        {
            Node* obj = touch->touchMonitors[i].ptr();
            if (obj && obj != target)
            {
                DisplayObject* dobj = dynamic_cast<DisplayObject*>(obj);
                if (dobj)
                {
                    if (dobj->isAncestorOf(target))
                        continue;
                }
                else
                {
                    GObject* gobj = dynamic_cast<GObject*>(obj);
                    if (gobj && gobj->displayObject() && gobj->displayObject()->isAncestorOf(target))
                        continue;
                }
                obj->dispatchEvent(UIEventType::TouchEnd);
            }
        }

        touch->touchMonitors.clear();

        target = touch->target.ptr<DisplayObject>();
    }
    if (target)
        target->bubbleEvent(UIEventType::TouchEnd);

    auto now = clock();
    float elapsed = (float)((now - touch->lastClickTime) / (double)CLOCKS_PER_SEC);

    if (elapsed < 0.45f)
    {
        if (touch->clickCount == 2)
            touch->clickCount = 1;
        else
            touch->clickCount++;
    }
    else
        touch->clickCount = 1;
    touch->lastClickTime = now;
}

void Stage::setMove(TouchInfo * touch)
{
    updateEvent(touch);

    if (touch->touchMonitors.size() > 0)
    {
        int len = touch->touchMonitors.size();
        for (int i = 0; i < len; i++)
        {
            Node* obj = touch->touchMonitors[i].ptr();
            if (obj != nullptr && obj->onStage())
                obj->dispatchEvent(UIEventType::TouchMove);
        }
    }

    dispatchEvent(UIEventType::TouchMove);
}

DisplayObject* Stage::clickTest(TouchInfo* touch)
{
    if (touch->downTargets.empty()
        || touch->clickCancelled
        || std::abs(touch->pos.x - touch->downPos.x) > 50 || std::abs(touch->pos.y - touch->downPos.y) > 50)
        return nullptr;

    DisplayObject* obj = touch->downTargets[0].ptr<DisplayObject>();
    if (obj && obj->onStage())
        return obj;

    obj = touch->target.ptr<DisplayObject>();
    while (obj != nullptr)
    {
        auto it = std::find(touch->downTargets.cbegin(), touch->downTargets.cend(), obj);
        if (it != touch->downTargets.cend() && it->onStage())
        {
            obj = it->ptr<DisplayObject>();
            break;
        }

        obj = obj->getParent();
    }

    return obj;
}

TouchInfo::TouchInfo() :
    touchId(-1),
    clickCount(0),
    mouseWheelDelta(0),
    button(-1),
    began(false),
    lastClickTime(0),
    clickCancelled(false),
    pos(0, 0),
    moved(false)
{
}

TouchInfo::~TouchInfo()
{
    downTargets.clear();
    touchMonitors.clear();
}

void TouchInfo::reset()
{
    touchId = -1;
    mouseWheelDelta = 0;
    button = -1;
    pos.setZero();
    downPos.setZero();
    clickCount = 0;
    lastClickTime = 0;
    began = false;
    downTargets.clear();
    lastRollOver = nullptr;
    clickCancelled = false;
    touchMonitors.clear();
}

NS_FGUI_END

