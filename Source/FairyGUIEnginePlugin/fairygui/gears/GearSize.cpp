#include "GearSize.h"
#include "GObject.h"
#include "UIPackage.h"
#include "GController.h"
#include "utils/ToolSet.h"
#include "utils/ActionUtils.h"

NS_FGUI_BEGIN

GearSize::GearSize(GObject * owner) :GearBase(owner)
{
}

GearSize::~GearSize()
{
}

void GearSize::init()
{
    _default = hkvVec4(_owner->getWidth(), _owner->getHeight(),
        _owner->getScaleX(), _owner->getScaleY());
    _storage.clear();
}

void GearSize::addStatus(const std::string&  pageId, const std::string& value)
{
    if (value == "-" || value.length() == 0)
        return;

    hkvVec4 v4(0, 0, 1, 1);
    ToolSet::splitString(value, ',', v4);

    if (pageId.size() == 0)
        _default = v4;
    else
        _storage[pageId] = v4;
}

void GearSize::apply()
{
    hkvVec4 gv;
    auto it = _storage.find(_controller->getSelectedPageId());
    if (it != _storage.end())
        gv = it->second;
    else
        gv = _default;

    if (tween && UIPackage::_constructing == 0 && !disableAllTweenEffect)
    {
        if (_owner->getActionByTag(InternalActionTag::GEAR_SIZE_ACTION) != nullptr)
        {
            if (_tweenTarget != gv)
            {
                _owner->stopActionByTag(InternalActionTag::GEAR_SIZE_ACTION);
                onTweenComplete();
            }
            else
                return;
        }

        bool a = gv.x != _owner->getWidth() || gv.y != _owner->getHeight();
        bool b = gv.z != _owner->getScaleX() || gv.w != _owner->getScaleY();
        if (a || b)
        {
            if (_owner->checkGearController(0, _controller))
                _displayLockToken = _owner->addDisplayLock();
            _tweenTarget = gv;

            ActionInterval* action = ActionVec4::create(tweenTime,
                hkvVec4(_owner->getWidth(), _owner->getHeight(), _owner->getScaleX(), _owner->getScaleY()),
                gv,
                CALLBACK_1(GearSize::onTweenUpdate, this, a, b));

            action = ActionUtils::composeActions(action, easeType, delay, CALLBACK_0(GearSize::onTweenComplete, this), InternalActionTag::GEAR_SIZE_ACTION);
            _owner->runAction(action);
        }
    }
    else
    {
        _owner->_gearLocked = true;
        _owner->setSize(gv.x, gv.y, _owner->checkGearController(1, _controller));
        _owner->setScale(gv.z, gv.w);
        _owner->_gearLocked = false;
    }
}

void GearSize::onTweenUpdate(const hkvVec4& v, bool a, bool b)
{
    _owner->_gearLocked = true;
    if (a)
        _owner->setSize(v.x, v.y, _owner->checkGearController(1, _controller));
    if (b)
        _owner->setScale(v.z, v.w);
    _owner->_gearLocked = false;
}

void GearSize::onTweenComplete()
{
    if (_displayLockToken != 0)
    {
        _owner->releaseDisplayLock(_displayLockToken);
        _displayLockToken = 0;
    }
    _owner->dispatchEvent(UIEventType::GearStop);
}

void GearSize::updateState()
{
    _storage[_controller->getSelectedPageId()] = hkvVec4(_owner->getWidth(), _owner->getHeight(),
        _owner->getScaleX(), _owner->getScaleY());
}

void GearSize::updateFromRelations(float dx, float dy)
{
    if (_controller != nullptr && !_storage.empty())
    {
        for (auto it = _storage.begin(); it != _storage.end(); ++it)
        {
            it->second = hkvVec4(it->second.x + dx, it->second.y + dy,
                it->second.z, it->second.w);
        }
        _default.x += dx;
        _default.y += dy;

        updateState();
    }
}


NS_FGUI_END