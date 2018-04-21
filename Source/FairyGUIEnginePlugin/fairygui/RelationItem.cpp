#include "RelationItem.h"
#include "GComponent.h"
#include "GGroup.h"

NS_FGUI_BEGIN

RelationItem::RelationItem(GObject* owner) :
    _target(nullptr)
{
    _owner = owner;
}

RelationItem::~RelationItem()
{
    releaseRefTarget(_target.ptr<GObject>());
}

void RelationItem::setTarget(GObject * value)
{
    GObject* old = _target.ptr<GObject>();
    if (old != value)
    {
        if (old)
            releaseRefTarget(old);
        _target = value;
        if (value)
            addRefTarget(value);
    }
}

void RelationItem::add(RelationType relationType, bool usePercent)
{
    if (relationType == RelationType::Size)
    {
        add(RelationType::Width, usePercent);
        add(RelationType::Height, usePercent);
        return;
    }

    for (auto &it : _defs)
    {
        if (it.type == relationType)
            return;
    }

    internalAdd(relationType, usePercent);
}

void RelationItem::internalAdd(RelationType relationType, bool usePercent)
{
    if (relationType == RelationType::Size)
    {
        internalAdd(RelationType::Width, usePercent);
        internalAdd(RelationType::Height, usePercent);
        return;
    }

    RelationDef info;
    info.percent = usePercent;
    info.type = relationType;
    _defs.push_back(info);

    if (usePercent || relationType == RelationType::Left_Center || relationType == RelationType::Center_Center || relationType == RelationType::Right_Center
        || relationType == RelationType::Top_Middle || relationType == RelationType::Middle_Middle || relationType == RelationType::Bottom_Middle)
        _owner->setPixelSnapping(true);
}

void RelationItem::remove(RelationType relationType)
{
    if (relationType == RelationType::Size)
    {
        remove(RelationType::Width);
        remove(RelationType::Height);
        return;
    }

    for (auto it = _defs.begin(); it != _defs.end(); ++it)
    {
        if (it->type == relationType)
        {
            _defs.erase(it);
            break;
        }
    }
}

void RelationItem::copyFrom(const RelationItem& source)
{
    setTarget(source._target.ptr<GObject>());

    _defs.clear();
    for (auto &it : source._defs)
        _defs.push_back(it);
}

bool RelationItem::isEmpty() const
{
    return _defs.size() == 0;
}

void RelationItem::applyOnSelfSizeChanged(float dWidth, float dHeight)
{
    if (_target == nullptr || _defs.size() == 0)
        return;

    float ox = _owner->_position.x;
    float oy = _owner->_position.y;

    for (auto &it : _defs)
    {
        switch (it.type)
        {
        case RelationType::Center_Center:
        case RelationType::Right_Center:
            _owner->setX(_owner->_position.x - dWidth / 2);
            break;

        case RelationType::Right_Left:
        case RelationType::Right_Right:
            _owner->setX(_owner->_position.x - dWidth);
            break;

        case RelationType::Middle_Middle:
        case RelationType::Bottom_Middle:
            _owner->setY(_owner->_position.y - dHeight / 2);
            break;
        case RelationType::Bottom_Top:
        case RelationType::Bottom_Bottom:
            _owner->setY(_owner->_position.y - dHeight);
            break;
        default:
            break;
        }
    }

    if (ox != _owner->_position.x || oy != _owner->_position.y)
    {
        ox = _owner->_position.x - ox;
        oy = _owner->_position.y - oy;

        _owner->updateGearFromRelations(1, ox, oy);

        if (_owner->_parent != nullptr)
        {
            const Vector<Transition*>& arr = _owner->getParent()->getTransitions();
            for (auto &it : arr)
                it->updateFromRelations(_owner->id, ox, oy);
        }
    }
}

void RelationItem::applyOnXYChanged(GObject* target, const RelationDef& info, float dx, float dy)
{
    switch (info.type)
    {
    case RelationType::Left_Left:
    case RelationType::Left_Center:
    case RelationType::Left_Right:
    case RelationType::Center_Center:
    case RelationType::Right_Left:
    case RelationType::Right_Center:
    case RelationType::Right_Right:
        _owner->setX(_owner->_position.x + dx);
        break;

    case RelationType::Top_Top:
    case RelationType::Top_Middle:
    case RelationType::Top_Bottom:
    case RelationType::Middle_Middle:
    case RelationType::Bottom_Top:
    case RelationType::Bottom_Middle:
    case RelationType::Bottom_Bottom:
        _owner->setY(_owner->_position.y + dy);
        break;

    case RelationType::Width:
    case RelationType::Height:
        break;

    case RelationType::LeftExt_Left:
    case RelationType::LeftExt_Right:
        _owner->setX(_owner->_position.x + dx);
        _owner->setWidth(_owner->_rawSize.x - dx);
        break;

    case RelationType::RightExt_Left:
    case RelationType::RightExt_Right:
        _owner->setWidth(_owner->_rawSize.x + dx);
        break;

    case RelationType::TopExt_Top:
    case RelationType::TopExt_Bottom:
        _owner->setY(_owner->_position.y + dy);
        _owner->setHeight(_owner->_rawSize.y - dy);
        break;

    case RelationType::BottomExt_Top:
    case RelationType::BottomExt_Bottom:
        _owner->setHeight(_owner->_rawSize.y + dy);
        break;
    default:
        break;
    }
}

void RelationItem::applyOnSizeChanged(GObject* target, const RelationDef& info)
{
    float targetX, targetY;
    if (target != _owner->_parent)
    {
        targetX = target->_position.x;
        targetY = target->_position.y;
    }
    else
    {
        targetX = 0;
        targetY = 0;
    }
    float v, tmp;

    switch (info.type)
    {
    case RelationType::Left_Left:
        if (info.percent && _target == _owner->_parent)
        {
            v = _owner->_position.x - targetX;
            if (info.percent)
                v = v / _targetData.z * target->_size.x;
            _owner->setX(targetX + v);
        }
        break;
    case RelationType::Left_Center:
        v = _owner->_position.x - (targetX + _targetData.z / 2);
        if (info.percent)
            v = v / _targetData.z * target->_size.x;
        _owner->setX(targetX + target->_size.x / 2 + v);
        break;
    case RelationType::Left_Right:
        v = _owner->_position.x - (targetX + _targetData.z);
        if (info.percent)
            v = v / _targetData.z * target->_size.x;
        _owner->setX(targetX + target->_size.x + v);
        break;
    case RelationType::Center_Center:
        v = _owner->_position.x + _owner->_rawSize.x / 2 - (targetX + _targetData.z / 2);
        if (info.percent)
            v = v / _targetData.z * target->_size.x;
        _owner->setX(targetX + target->_size.x / 2 + v - _owner->_rawSize.x / 2);
        break;
    case RelationType::Right_Left:
        v = _owner->_position.x + _owner->_rawSize.x - targetX;
        if (info.percent)
            v = v / _targetData.z * target->_size.x;
        _owner->setX(targetX + v - _owner->_rawSize.x);
        break;
    case RelationType::Right_Center:
        v = _owner->_position.x + _owner->_rawSize.x - (targetX + _targetData.z / 2);
        if (info.percent)
            v = v / _targetData.z * target->_size.x;
        _owner->setX(targetX + target->_size.x / 2 + v - _owner->_rawSize.x);
        break;
    case RelationType::Right_Right:
        v = _owner->_position.x + _owner->_rawSize.x - (targetX + _targetData.z);
        if (info.percent)
            v = v / _targetData.z * target->_size.x;
        _owner->setX(targetX + target->_size.x + v - _owner->_rawSize.x);
        break;

    case RelationType::Top_Top:
        if (info.percent && _target == _owner->_parent)
        {
            v = _owner->_position.y - targetY;
            if (info.percent)
                v = v / _targetData.w * target->_size.y;
            _owner->setY(targetY + v);
        }
        break;
    case RelationType::Top_Middle:
        v = _owner->_position.y - (targetY + _targetData.w / 2);
        if (info.percent)
            v = v / _targetData.w * target->_size.y;
        _owner->setY(targetY + target->_size.y / 2 + v);
        break;
    case RelationType::Top_Bottom:
        v = _owner->_position.y - (targetY + _targetData.w);
        if (info.percent)
            v = v / _targetData.w * target->_size.y;
        _owner->setY(targetY + target->_size.y + v);
        break;
    case RelationType::Middle_Middle:
        v = _owner->_position.y + _owner->_rawSize.y / 2 - (targetY + _targetData.w / 2);
        if (info.percent)
            v = v / _targetData.w * target->_size.y;
        _owner->setY(targetY + target->_size.y / 2 + v - _owner->_rawSize.y / 2);
        break;
    case RelationType::Bottom_Top:
        v = _owner->_position.y + _owner->_rawSize.y - targetY;
        if (info.percent)
            v = v / _targetData.w * target->_size.y;
        _owner->setY(targetY + v - _owner->_rawSize.y);
        break;
    case RelationType::Bottom_Middle:
        v = _owner->_position.y + _owner->_rawSize.y - (targetY + _targetData.w / 2);
        if (info.percent)
            v = v / _targetData.w * target->_size.y;
        _owner->setY(targetY + target->_size.y / 2 + v - _owner->_rawSize.y);
        break;
    case RelationType::Bottom_Bottom:
        v = _owner->_position.y + _owner->_rawSize.y - (targetY + _targetData.w);
        if (info.percent)
            v = v / _targetData.w * target->_size.y;
        _owner->setY(targetY + target->_size.y + v - _owner->_rawSize.y);
        break;

    case RelationType::Width:
        if (_owner->_underConstruct && _owner == target->_parent)
            v = _owner->sourceSize.x - target->initSize.x;
        else
            v = _owner->_rawSize.x - _targetData.z;
        if (info.percent)
            v = v / _targetData.z * target->_size.x;
        if (_target == _owner->_parent)
            _owner->setSize(target->_size.x + v, _owner->_rawSize.y, true);
        else
            _owner->setWidth(target->_size.x + v);
        break;
    case RelationType::Height:
        if (_owner->_underConstruct && _owner == target->_parent)
            v = _owner->sourceSize.y - target->initSize.y;
        else
            v = _owner->_rawSize.y - _targetData.w;
        if (info.percent)
            v = v / _targetData.w * target->_size.y;
        if (_target == _owner->_parent)
            _owner->setSize(_owner->_rawSize.x, target->_size.y + v, true);
        else
            _owner->setHeight(target->_size.y + v);
        break;

    case RelationType::LeftExt_Left:
        break;
    case RelationType::LeftExt_Right:
        v = _owner->_position.x - (targetX + _targetData.z);
        if (info.percent)
            v = v / _targetData.z * target->_size.x;
        tmp = _owner->_position.x;
        _owner->setX(targetX + target->_size.x + v);
        _owner->setWidth(_owner->_rawSize.x - (_owner->_position.x - tmp));
        break;
    case RelationType::RightExt_Left:
        break;
    case RelationType::RightExt_Right:
        if (_owner->_underConstruct && _owner == target->_parent)
            v = _owner->sourceSize.x - (targetX + target->initSize.x);
        else
            v = _owner->_rawSize.x - (targetX + _targetData.z);
        if (_owner != target->_parent)
            v += _owner->_position.x;
        if (info.percent)
            v = v / _targetData.z * target->_size.x;
        if (_owner != target->_parent)
            _owner->setWidth(targetX + target->_size.x + v - _owner->_position.x);
        else
            _owner->setWidth(targetX + target->_size.x + v);
        break;
    case RelationType::TopExt_Top:
        break;
    case RelationType::TopExt_Bottom:
        v = _owner->_position.y - (targetY + _targetData.w);
        if (info.percent)
            v = v / _targetData.w * target->_size.y;
        tmp = _owner->_position.y;
        _owner->setY(targetY + target->_size.y + v);
        _owner->setHeight(_owner->_rawSize.y - (_owner->_position.y - tmp));
        break;
    case RelationType::BottomExt_Top:
        break;
    case RelationType::BottomExt_Bottom:
        if (_owner->_underConstruct && _owner == target->_parent)
            v = _owner->sourceSize.y - (targetY + target->initSize.y);
        else
            v = _owner->_rawSize.y - (targetY + _targetData.w);
        if (_owner != target->_parent)
            v += _owner->_position.y;
        if (info.percent)
            v = v / _targetData.w * target->_size.y;
        if (_owner != target->_parent)
            _owner->setHeight(targetY + target->_size.y + v - _owner->_position.y);
        else
            _owner->setHeight(targetY + target->_size.y + v);
        break;
    default:
        break;
    }
}

void RelationItem::addRefTarget(GObject* target)
{
    if (!target)
        return;

    if (target != _owner->_parent)
        target->addListener(UIEventType::PositionChange, CALLBACK_1(RelationItem::onTargetXYChanged, this), EventTag(this));
    target->addListener(UIEventType::SizeChange, CALLBACK_1(RelationItem::onTargetSizeChanged, this), EventTag(this));

    _targetData.x = target->_position.x;
    _targetData.y = target->_position.y;
    _targetData.z = target->_size.x;
    _targetData.w = target->_size.y;
}

void RelationItem::releaseRefTarget(GObject* target)
{
    if (!target)
        return;

    target->removeListener(UIEventType::PositionChange, EventTag(this));
    target->removeListener(UIEventType::SizeChange, EventTag(this));
}

void RelationItem::onTargetXYChanged(EventContext* context)
{
    GObject* target = dynamic_cast<GObject*>(context->getSender());
    if (_owner->relations()->handling != nullptr
        || (_owner->_group != nullptr && _owner->_group->_updating != 0))
    {
        _targetData.x = target->_position.x;
        _targetData.y = target->_position.y;
        return;
    }

    _owner->relations()->handling = target;

    float ox = _owner->_position.x;
    float oy = _owner->_position.y;
    float dx = target->_position.x - _targetData.x;
    float dy = target->_position.y - _targetData.y;

    for (auto &it : _defs)
        applyOnXYChanged(target, it, dx, dy);

    _targetData.x = target->_position.x;
    _targetData.y = target->_position.y;

    if (ox != _owner->_position.x || oy != _owner->_position.y)
    {
        ox = _owner->_position.x - ox;
        oy = _owner->_position.y - oy;

        _owner->updateGearFromRelations(1, ox, oy);

        if (_owner->_parent != nullptr)
        {
            const Vector<Transition*>& arr = _owner->getParent()->getTransitions();
            for (auto &it : arr)
                it->updateFromRelations(_owner->id, ox, oy);
        }
    }

    _owner->relations()->handling = nullptr;
}

void RelationItem::onTargetSizeChanged(EventContext* context)
{
    GObject* target = dynamic_cast<GObject*>(context->getSender());
    if (_owner->relations()->handling != nullptr
        || (_owner->_group != nullptr && _owner->_group->_updating != 0))
    {
        _targetData.z = target->_size.x;
        _targetData.w = target->_size.y;
        return;
    }

    _owner->relations()->handling = target;

    float ox = _owner->_position.x;
    float oy = _owner->_position.y;
    float ow = _owner->_rawSize.x;
    float oh = _owner->_rawSize.y;

    for (auto &it : _defs)
        applyOnSizeChanged(target, it);

    _targetData.z = target->_size.x;
    _targetData.w = target->_size.y;

    if (ox != _owner->_position.x || oy != _owner->_position.y)
    {
        ox = _owner->_position.x - ox;
        oy = _owner->_position.y - oy;

        _owner->updateGearFromRelations(1, ox, oy);

        if (_owner->_parent != nullptr)
        {
            const Vector<Transition*>& arr = _owner->getParent()->getTransitions();
            for (auto &it : arr)
                it->updateFromRelations(_owner->id, ox, oy);
        }
    }

    if (ow != _owner->_rawSize.x || oh != _owner->_rawSize.y)
    {
        ow = _owner->_rawSize.x - ow;
        oh = _owner->_rawSize.y - oh;

        _owner->updateGearFromRelations(2, ow, oh);
    }

    _owner->relations()->handling = nullptr;
}

NS_FGUI_END