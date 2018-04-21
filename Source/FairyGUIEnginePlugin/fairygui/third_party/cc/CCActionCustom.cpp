/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2017 Chukong Technologies Inc.
 
http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "CCActionCustom.h"
#include "CCActionInstant.h"

#include <stdarg.h>

NS_CC_BEGIN

ActionFloat2* ActionFloat2::create(float duration, const hkvVec2& from, const hkvVec2& to, ActionFloat2Callback onUpdate)
{
    auto ref = new /*(std::nothrow)*/ ActionFloat2();
    if (ref && ref->initWithDuration(duration, from, to, onUpdate))
    {
        ref->autorelease();
        return ref;
    }

    delete ref;
    return nullptr;
}

bool ActionFloat2::initWithDuration(float duration, const hkvVec2& from, const hkvVec2& to, ActionFloat2Callback onUpdate)
{
    if (ActionInterval::initWithDuration(duration))
    {
        _from = from;
        _to = to;
        _updateCallback = onUpdate;
        return true;
    }
    return false;
}

ActionFloat2* ActionFloat2::clone() const
{
    return ActionFloat2::create(_duration, _from, _to, _updateCallback);
}

void ActionFloat2::startWithTarget(Node *target)
{
    ActionInterval::startWithTarget(target);
    _delta = _to - _from;
}

void ActionFloat2::update(float delta)
{
    hkvVec2 value = _to - _delta * (1 - delta);

    if (_updateCallback)
    {
        // report back value to caller
        _updateCallback(value.x, value.y);
    }
}

ActionFloat2* ActionFloat2::reverse() const
{
    return ActionFloat2::create(_duration, _to, _from, _updateCallback);
}

//------------------

ActionVec2* ActionVec2::create(float duration, const hkvVec2& from, const hkvVec2& to, ActionVec2Callback onUpdate)
{
    auto ref = new /*(std::nothrow)*/ ActionVec2();
    if (ref && ref->initWithDuration(duration, from, to, onUpdate))
    {
        ref->autorelease();
        return ref;
    }

    delete ref;
    return nullptr;
}

bool ActionVec2::initWithDuration(float duration, const hkvVec2& from, const hkvVec2& to, ActionVec2Callback onUpdate)
{
    if (ActionInterval::initWithDuration(duration))
    {
        _from = from;
        _to = to;
        _updateCallback = onUpdate;
        return true;
    }
    return false;
}

ActionVec2* ActionVec2::clone() const
{
    return ActionVec2::create(_duration, _from, _to, _updateCallback);
}

void ActionVec2::startWithTarget(Node *target)
{
    ActionInterval::startWithTarget(target);
    _delta = _to - _from;
}

void ActionVec2::update(float delta)
{
    hkvVec2 value = _to - _delta * (1 - delta);

    if (_updateCallback)
    {
        // report back value to caller
        _updateCallback(value);
    }
}

ActionVec2* ActionVec2::reverse() const
{
    return ActionVec2::create(_duration, _to, _from, _updateCallback);
}

//--------------------

ActionVec4* ActionVec4::create(float duration, const hkvVec4& from, const hkvVec4& to, ActionVec4Callback onUpdate)
{
    auto ref = new /*(std::nothrow)*/ ActionVec4();
    if (ref && ref->initWithDuration(duration, from, to, onUpdate))
    {
        ref->autorelease();
        return ref;
    }

    delete ref;
    return nullptr;
}

bool ActionVec4::initWithDuration(float duration, const hkvVec4& from, const hkvVec4& to, ActionVec4Callback onUpdate)
{
    if (ActionInterval::initWithDuration(duration))
    {
        _from = from;
        _to = to;
        _updateCallback = onUpdate;
        return true;
    }
    return false;
}

ActionVec4* ActionVec4::clone() const
{
    return ActionVec4::create(_duration, _from, _to, _updateCallback);
}

void ActionVec4::startWithTarget(Node *target)
{
    ActionInterval::startWithTarget(target);
    _delta = _to - _from;
}

void ActionVec4::update(float delta)
{
    hkvVec4 value = _to - _delta * (1 - delta);

    if (_updateCallback)
    {
        // report back value to caller
        _updateCallback(value);
    }
}

ActionVec4* ActionVec4::reverse() const
{
    return ActionVec4::create(_duration, _to, _from, _updateCallback);
}

//-------------------

RepeatYoyo * RepeatYoyo::create(FiniteTimeAction * action, unsigned int times, bool yoyo)
{
    RepeatYoyo* repeat = new /*(std::nothrow)*/ RepeatYoyo();
    if (repeat && repeat->initWithAction(action, times))
    {
        repeat->_yoyo = yoyo;
        repeat->autorelease();
        return repeat;
    }

    delete repeat;
    return nullptr;
}

RepeatYoyo * RepeatYoyo::clone() const
{
    return RepeatYoyo::create(_innerAction->clone(), _times, _yoyo);
}

RepeatYoyo* RepeatYoyo::reverse() const
{
    return RepeatYoyo::create(_innerAction->reverse(), _times, _yoyo);
}

void RepeatYoyo::update(float dt)
{
    if (dt >= _nextDt)
    {
        while (dt >= _nextDt && _total < _times)
        {
            float dt2 = (_yoyo && _total % 2 == 1) ? 0 : 1.0f;
            if (!(sendUpdateEventToScript(dt2, _innerAction)))
                _innerAction->update(dt2);
            _total++;

            _innerAction->stop();

            _innerAction->startWithTarget(_target);
            _nextDt = _innerAction->getDuration() / _duration * (_total + 1);
        }

        // fix for issue #1288, incorrect end value of repeat
        if (std::abs(dt - 1.0f) < FLT_EPSILON && _total < _times)
        {
            float dt2 = (_yoyo && _total % 2 == 1) ? 0 : 1.0f;
            if (!(sendUpdateEventToScript(dt2, _innerAction)))
                _innerAction->update(dt2);

            _total++;
        }

        // don't set an instant action back or update it, it has no use because it has no duration
        if (!_actionInstant)
        {
            if (_total == _times)
            {
                // minggo: inner action update is invoked above, don't have to invoke it here
                //                if (!(sendUpdateEventToScript(1, _innerAction)))
                //                    _innerAction->update(1);
                _innerAction->stop();
            }
            else
            {
                float dt2 = dt - (_nextDt - _innerAction->getDuration() / _duration);
                if (_yoyo && _total % 2 == 1)
                    dt2 = 1 - dt2;

                // issue #390 prevent jerk, use right update
                if (!(sendUpdateEventToScript(dt2, _innerAction)))
                    _innerAction->update(dt2);
            }
        }
    }
    else
    {
        float dt2 = fmodf(dt * _times, 1.0f);
        if (_yoyo && _total % 2 == 1)
            dt2 = 1 - dt2;
        if (!(sendUpdateEventToScript(dt2, _innerAction)))
            _innerAction->update(dt2);
    }
}

NS_CC_END
