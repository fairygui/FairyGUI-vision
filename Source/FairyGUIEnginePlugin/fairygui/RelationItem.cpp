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


void RelationItem::applyOnSelfSizeChanged(float dWidth, float dHeight, bool applyPivot)
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
            _owner->setX(_owner->_position.x - (0.5 - (applyPivot ? _owner->_pivot.x : 0)) * dWidth);

        case RelationType::Right_Center:
        case RelationType::Right_Left:
        case RelationType::Right_Right:
            _owner->setX(_owner->_position.x - (1 - (applyPivot ? _owner->_pivot.x : 0)) * dWidth);
            break;

        case RelationType::Middle_Middle:
            _owner->setY(_owner->_position.y - (0.5 - (applyPivot ? _owner->_pivot.y : 0)) * dHeight);

        case RelationType::Bottom_Middle:
        case RelationType::Bottom_Top:
        case RelationType::Bottom_Bottom:
            _owner->setY(_owner->_position.y - (1 - (applyPivot ? _owner->_pivot.y : 0)) * dHeight);
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
            const Vector<Transition*>& arr = _owner->_parent->getTransitions();
            for (auto &it : arr)
                it->updateFromRelations(_owner->id, ox, oy);
        }
    }
}

void RelationItem::applyOnXYChanged(GObject* target, const RelationDef& info, float dx, float dy)
{
    float tmp;

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
        tmp = _owner->getXMin();
        _owner->setWidth(_owner->_rawSize.x - dx);
        _owner->setXMin(tmp + dx);
        break;

    case RelationType::RightExt_Left:
    case RelationType::RightExt_Right:
        tmp = _owner->getXMin();
        _owner->setWidth(_owner->_rawSize.x + dx);
        _owner->setXMin(tmp);
        break;

    case RelationType::TopExt_Top:
    case RelationType::TopExt_Bottom:
        tmp = _owner->getYMin();
        _owner->setHeight(_owner->_rawSize.y - dy);
        _owner->setYMin(tmp + dy);
        break;

    case RelationType::BottomExt_Top:
    case RelationType::BottomExt_Bottom:
        tmp = _owner->getYMin();
        _owner->setHeight(_owner->_rawSize.y + dy);
        _owner->setYMin(tmp);
        break;

    default:
        break;
    }
}

void RelationItem::applyOnSizeChanged(GObject* target, const RelationDef& info)
{
    float pos = 0, pivot = 0, delta = 0;
    if (info.axis == 0)
    {
        if (target != _owner->_parent)
        {
            pos = target->_position.x;
            if (target->_pivotAsAnchor)
                pivot = target->_pivot.x;
        }

        if (info.percent)
        {
            if (_targetData.z != 0)
                delta = target->_size.x / _targetData.z;
        }
        else
            delta = target->_size.x - _targetData.z;
    }
    else
    {
        if (target != _owner->_parent)
        {
            pos = target->_position.y;
            if (target->_pivotAsAnchor)
                pivot = target->_pivot.y;
        }

        if (info.percent)
        {
            if (_targetData.w != 0)
                delta = target->_size.y / _targetData.w;
        }
        else
            delta = target->_size.y - _targetData.w;
    }

    float v, tmp;

    switch (info.type)
    {
    case RelationType::Left_Left:
        if (info.percent)
            _owner->setXMin(pos + (_owner->getXMin() - pos) * delta);
        else if (pivot != 0)
            _owner->setX(_owner->_position.x + delta * (-pivot));
        break;
    case RelationType::Left_Center:
        if (info.percent)
            _owner->setXMin(pos + (_owner->getXMin() - pos) * delta);
        else
            _owner->setX(_owner->_position.x + delta * (0.5f - pivot));
        break;
    case RelationType::Left_Right:
        if (info.percent)
            _owner->setXMin(pos + (_owner->getXMin() - pos) * delta);
        else
            _owner->setX(_owner->_position.x + delta * (1 - pivot));
        break;
    case RelationType::Center_Center:
        if (info.percent)
            _owner->setXMin(pos + (_owner->getXMin() + _owner->_rawSize.x * 0.5f - pos) * delta - _owner->_rawSize.x * 0.5f);
        else
            _owner->setX(_owner->_position.x + delta * (0.5f - pivot));
        break;
    case RelationType::Right_Left:
        if (info.percent)
            _owner->setXMin(pos + (_owner->getXMin() + _owner->_rawSize.x - pos) * delta - _owner->_rawSize.x);
        else if (pivot != 0)
            _owner->setX(_owner->_position.x + delta * (-pivot));
        break;
    case RelationType::Right_Center:
        if (info.percent)
            _owner->setXMin(pos + (_owner->getXMin() + _owner->_rawSize.x - pos) * delta - _owner->_rawSize.x);
        else
            _owner->setX(_owner->_position.x + delta * (0.5f - pivot));
        break;
    case RelationType::Right_Right:
        if (info.percent)
            _owner->setXMin(pos + (_owner->getXMin() + _owner->_rawSize.x - pos) * delta - _owner->_rawSize.x);
        else
            _owner->setX(_owner->_position.x + delta * (1 - pivot));
        break;

    case RelationType::Top_Top:
        if (info.percent)
            _owner->setYMin(pos + (_owner->getYMin() - pos) * delta);
        else if (pivot != 0)
            _owner->setY(_owner->_position.y + delta * (-pivot));
        break;
    case RelationType::Top_Middle:
        if (info.percent)
            _owner->setYMin(pos + (_owner->getYMin() - pos) * delta);
        else
            _owner->setY(_owner->_position.y + delta * (0.5f - pivot));
        break;
    case RelationType::Top_Bottom:
        if (info.percent)
            _owner->setYMin(pos + (_owner->getYMin() - pos) * delta);
        else
            _owner->setY(_owner->_position.y + delta * (1 - pivot));
        break;
    case RelationType::Middle_Middle:
        if (info.percent)
            _owner->setYMin(pos + (_owner->getYMin() + _owner->_rawSize.y * 0.5f - pos) * delta - _owner->_rawSize.y * 0.5f);
        else
            _owner->setY(_owner->_position.y + delta * (0.5f - pivot));
        break;
    case RelationType::Bottom_Top:
        if (info.percent)
            _owner->setYMin(pos + (_owner->getYMin() + _owner->_rawSize.y - pos) * delta - _owner->_rawSize.y);
        else if (pivot != 0)
            _owner->setY(_owner->_position.y + delta * (-pivot));
        break;
    case RelationType::Bottom_Middle:
        if (info.percent)
            _owner->setYMin(pos + (_owner->getYMin() + _owner->_rawSize.y - pos) * delta - _owner->_rawSize.y);
        else
            _owner->setY(_owner->_position.y + delta * (0.5f - pivot));
        break;
    case RelationType::Bottom_Bottom:
        if (info.percent)
            _owner->setYMin(pos + (_owner->getYMin() + _owner->_rawSize.y - pos) * delta - _owner->_rawSize.y);
        else
            _owner->setY(_owner->_position.y + delta * (1 - pivot));
        break;

    case RelationType::Width:
        if (_owner->_underConstruct && _owner == target->_parent)
            v = _owner->sourceSize.x - target->initSize.x;
        else
            v = _owner->_rawSize.x - _targetData.z;
        if (info.percent)
            v = v * delta;
        if (_target == _owner->_parent)
        {
            if (_owner->_pivotAsAnchor)
            {
                tmp = _owner->getXMin();
                _owner->setSize(target->_size.x + v, _owner->_rawSize.y, true);
                _owner->setXMin(tmp);
            }
            else
                _owner->setSize(target->_size.x + v, _owner->_rawSize.y, true);
        }
        else
            _owner->setWidth(target->_size.x + v);
        break;
    case RelationType::Height:
        if (_owner->_underConstruct && _owner == target->_parent)
            v = _owner->sourceSize.y - target->initSize.y;
        else
            v = _owner->_rawSize.y - _targetData.w;
        if (info.percent)
            v = v * delta;
        if (_target == _owner->_parent)
        {
            if (_owner->_pivotAsAnchor)
            {
                tmp = _owner->getYMin();
                _owner->setSize(_owner->_rawSize.x, target->_size.y + v, true);
                _owner->setYMin(tmp);
            }
            else
                _owner->setSize(_owner->_rawSize.x, target->_size.y + v, true);
        }
        else
            _owner->setHeight(target->_size.y + v);
        break;

    case RelationType::LeftExt_Left:
        tmp = _owner->getXMin();
        if (info.percent)
            v = pos + (tmp - pos) * delta - tmp;
        else
            v = delta * (-pivot);
        _owner->setWidth(_owner->_rawSize.x - v);
        _owner->setXMin(tmp + v);
        break;
    case RelationType::LeftExt_Right:
        tmp = _owner->getXMin();
        if (info.percent)
            v = pos + (tmp - pos) * delta - tmp;
        else
            v = delta * (1 - pivot);
        _owner->setWidth(_owner->_rawSize.x - v);
        _owner->setXMin(tmp + v);
        break;
    case RelationType::RightExt_Left:
        tmp = _owner->getXMin();
        if (info.percent)
            v = pos + (tmp + _owner->_rawSize.x - pos) * delta - (tmp + _owner->_rawSize.x);
        else
            v = delta * (-pivot);
        _owner->setWidth(_owner->_rawSize.x + v);
        _owner->setXMin(tmp);
        break;
    case RelationType::RightExt_Right:
        tmp = _owner->getXMin();
        if (info.percent)
        {
            if (_owner == target->_parent)
            {
                if (_owner->_underConstruct)
                    _owner->setWidth(pos + target->_size.x - target->_size.x * pivot +
                    (_owner->sourceSize.x - pos - target->initSize.x + target->initSize.x * pivot) * delta);
                else
                    _owner->setWidth(pos + (_owner->_rawSize.x - pos) * delta);
            }
            else
            {
                v = pos + (tmp + _owner->_rawSize.x - pos) * delta - (tmp + _owner->_rawSize.x);
                _owner->setWidth(_owner->_rawSize.x + v);
                _owner->setXMin(tmp);
            }
        }
        else
        {
            if (_owner == target->_parent)
            {
                if (_owner->_underConstruct)
                    _owner->setWidth(_owner->sourceSize.x + (target->_size.x - target->initSize.x) * (1 - pivot));
                else
                    _owner->setWidth(_owner->_rawSize.x + delta * (1 - pivot));
            }
            else
            {
                v = delta * (1 - pivot);
                _owner->setWidth(_owner->_rawSize.x + v);
                _owner->setXMin(tmp);
            }
        }
        break;
    case RelationType::TopExt_Top:
        tmp = _owner->getYMin();
        if (info.percent)
            v = pos + (tmp - pos) * delta - tmp;
        else
            v = delta * (-pivot);
        _owner->setHeight(_owner->_rawSize.y - v);
        _owner->setYMin(tmp + v);
        break;
    case RelationType::TopExt_Bottom:
        tmp = _owner->getYMin();
        if (info.percent)
            v = pos + (tmp - pos) * delta - tmp;
        else
            v = delta * (1 - pivot);
        _owner->setHeight(_owner->_rawSize.y - v);
        _owner->setYMin(tmp + v);
        break;
    case RelationType::BottomExt_Top:
        tmp = _owner->getYMin();
        if (info.percent)
            v = pos + (tmp + _owner->_rawSize.y - pos) * delta - (tmp + _owner->_rawSize.y);
        else
            v = delta * (-pivot);
        _owner->setHeight(_owner->_rawSize.y + v);
        _owner->setYMin(tmp);
        break;
    case RelationType::BottomExt_Bottom:
        tmp = _owner->getYMin();
        if (info.percent)
        {
            if (_owner == target->_parent)
            {
                if (_owner->_underConstruct)
                    _owner->setHeight(pos + target->_size.y - target->_size.y * pivot +
                    (_owner->sourceSize.y - pos - target->initSize.y + target->initSize.y * pivot) * delta);
                else
                    _owner->setHeight(pos + (_owner->_rawSize.y - pos) * delta);
            }
            else
            {
                v = pos + (tmp + _owner->_rawSize.y - pos) * delta - (tmp + _owner->_rawSize.y);
                _owner->setHeight(_owner->_rawSize.y + v);
                _owner->setYMin(tmp);
            }
        }
        else
        {
            if (_owner == target->_parent)
            {
                if (_owner->_underConstruct)
                    _owner->setHeight(_owner->sourceSize.y + (target->_size.y - target->initSize.y) * (1 - pivot));
                else
                    _owner->setHeight(_owner->_rawSize.y + delta * (1 - pivot));
            }
            else
            {
                v = delta * (1 - pivot);
                _owner->setHeight(_owner->_rawSize.y + v);
                _owner->setYMin(tmp);
            }
        }
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