/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2010-2012 cocos2d-x.org
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

#ifndef __ACTION_CCINTERVAL_ACTION2_H__
#define __ACTION_CCINTERVAL_ACTION2_H__

#include <vector>

#include "CCActionInterval.h"

NS_CC_BEGIN

class CC_DLL ActionFloat2 : public ActionInterval
{
public:
    typedef std::function<void(float x, float y)> ActionFloat2Callback;
    static ActionFloat2* create(float duration, const hkvVec2& from, const hkvVec2& to, ActionFloat2Callback onUpdate);
    void startWithTarget(Node* target) override;
    void update(float delta) override;
    ActionFloat2* reverse() const override;
    ActionFloat2* clone() const override;

CC_CONSTRUCTOR_ACCESS:
    ActionFloat2() {};
    virtual ~ActionFloat2() {};

    bool initWithDuration(float duration, const hkvVec2& from, const hkvVec2& to, ActionFloat2Callback onUpdate);

protected:
    hkvVec2 _from;
    hkvVec2 _to;
    hkvVec2 _delta;
    ActionFloat2Callback _updateCallback;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(ActionFloat2);
};

class CC_DLL ActionVec2 : public ActionInterval
{
public:
    typedef std::function<void(const hkvVec2& value)> ActionVec2Callback;
    static ActionVec2* create(float duration, const hkvVec2& from, const hkvVec2& to, ActionVec2Callback onUpdate);
    void startWithTarget(Node* target) override;
    void update(float delta) override;
    ActionVec2* reverse() const override;
    ActionVec2* clone() const override;

CC_CONSTRUCTOR_ACCESS:
    ActionVec2() {};
    virtual ~ActionVec2() {};

    bool initWithDuration(float duration, const hkvVec2& from, const hkvVec2& to, ActionVec2Callback onUpdate);

protected:
    hkvVec2 _from;
    hkvVec2 _to;
    hkvVec2 _delta;
    ActionVec2Callback _updateCallback;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(ActionVec2);
};

class CC_DLL ActionVec4 : public ActionInterval
{
public:
    typedef std::function<void(const hkvVec4& value)> ActionVec4Callback;
    static ActionVec4* create(float duration, const hkvVec4& from, const hkvVec4& to, ActionVec4Callback onUpdate);
    void startWithTarget(Node* target) override;
    void update(float delta) override;
    ActionVec4* reverse() const override;
    ActionVec4* clone() const override;

CC_CONSTRUCTOR_ACCESS:
    ActionVec4() {};
    virtual ~ActionVec4() {};

    bool initWithDuration(float duration, const hkvVec4& from, const hkvVec4& to, ActionVec4Callback onUpdate);

protected:
    hkvVec4 _from;
    hkvVec4 _to;
    hkvVec4 _delta;
    ActionVec4Callback _updateCallback;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(ActionVec4);
};

class CC_DLL RepeatYoyo : public Repeat
{
public:
    static RepeatYoyo* create(FiniteTimeAction *action, unsigned int times, bool yoyo);
    virtual RepeatYoyo* clone() const override;
    virtual RepeatYoyo* reverse() const override;
    virtual void update(float dt) override;

    RepeatYoyo() {}

protected:
    bool _yoyo;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(RepeatYoyo);
};

// end of actions group
/// @}

NS_CC_END

#endif //__ACTION_CCINTERVAL_ACTION_H__
