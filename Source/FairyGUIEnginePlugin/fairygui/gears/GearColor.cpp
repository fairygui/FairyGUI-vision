#include "GearColor.h"
#include "GObject.h"
#include "UIPackage.h"
#include "GController.h"
#include "utils/ToolSet.h"
#include "utils/ActionUtils.h"

NS_FGUI_BEGIN

GearColor::GearColorValue::GearColorValue()
{

}

GearColor::GearColorValue::GearColorValue(const VColorRef& color, const VColorRef& strokeColor)
{
    this->color = color;
    this->outlineColor = strokeColor;
}

GearColor::GearColor(GObject * owner) :GearBase(owner)
{

}

GearColor::~GearColor()
{
}

void GearColor::init()
{
    IColorGear *cg = dynamic_cast<IColorGear*>(_owner);

    _default = GearColorValue(cg->getColor(), cg->getOutlineColor());
    _storage.clear();
}

void GearColor::addStatus(const std::string&  pageId, const std::string& value)
{
    if (value == "-" || value.length() == 0)
        return;

    std::vector<std::string> arr;
    ToolSet::splitString(value, ',', arr);

    GearColorValue gv;
    gv.color = ToolSet::convertFromHtmlColor(arr[0].c_str());
    if (arr.size() == 1)
        gv.outlineColor = VColorRef(0, 0, 0, 0);
    else
        gv.outlineColor = ToolSet::convertFromHtmlColor(arr[1].c_str());

    if (pageId.size() == 0)
        _default = gv;
    else
        _storage[pageId] = gv;
}

void GearColor::apply()
{
    GearColorValue gv;
    auto it = _storage.find(_controller->getSelectedPageId());
    if (it != _storage.end())
        gv = it->second;
    else
        gv = _default;

    IColorGear *cg = dynamic_cast<IColorGear*>(_owner);

    if (tween && UIPackage::_constructing == 0 && !disableAllTweenEffect)
    {
        if (gv.outlineColor.a > 0)
        {
            _owner->_gearLocked = true;
            cg->setOutlineColor(gv.outlineColor);
            _owner->_gearLocked = false;
        }

        if (_owner->getActionByTag(InternalActionTag::GEAR_COLOR_ACTION) != nullptr)
        {
            if (_tweenTarget.x != gv.color.r || _tweenTarget.y != gv.color.g || _tweenTarget.z != gv.color.b)
            {
                _owner->stopActionByTag(InternalActionTag::GEAR_COLOR_ACTION);
                onTweenComplete();
            }
            else
                return;
        }

        if (gv.color != cg->getColor())
        {
            if (_owner->checkGearController(0, _controller))
                _displayLockToken = _owner->addDisplayLock();
            _tweenTarget.set(gv.color.r, gv.color.g, gv.color.b, gv.color.a);
            const VColorRef& curColor = cg->getColor();

            ActionInterval* action = ActionVec4::create(tweenTime,
                hkvVec4(curColor.r, curColor.g, curColor.b, curColor.a),
                _tweenTarget,
                CALLBACK_1(GearColor::onTweenUpdate, this));
            action = ActionUtils::composeActions(action, easeType, delay, CALLBACK_0(GearColor::onTweenComplete, this), InternalActionTag::GEAR_COLOR_ACTION);
            _owner->runAction(action);
        }
    }
    else
    {
        _owner->_gearLocked = true;
        cg->setColor(gv.color);
        if (gv.outlineColor.a > 0)
            cg->setOutlineColor(gv.outlineColor);
        _owner->_gearLocked = false;
    }
}

void GearColor::onTweenUpdate(const hkvVec4& v)
{
    IColorGear *cg = dynamic_cast<IColorGear*>(_owner);

    _owner->_gearLocked = true;
    cg->setColor(VColorRef((UBYTE)v.x, (UBYTE)v.y, (UBYTE)v.z, (UBYTE)v.w));
    _owner->_gearLocked = false;
}

void GearColor::onTweenComplete()
{
    if (_displayLockToken != 0)
    {
        _owner->releaseDisplayLock(_displayLockToken);
        _displayLockToken = 0;
    }
    _owner->dispatchEvent(UIEventType::GearStop);
}

void GearColor::updateState()
{
    IColorGear *cg = dynamic_cast<IColorGear*>(_owner);
    _storage[_controller->getSelectedPageId()] = GearColorValue(cg->getColor(), cg->getOutlineColor());
}

NS_FGUI_END