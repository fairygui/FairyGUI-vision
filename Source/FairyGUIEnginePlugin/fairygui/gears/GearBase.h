#ifndef __GEARBASE_H__
#define __GEARBASE_H__

#include "FGUIMacros.h"
#include "third_party/cc/CCTweenFunction.h"

NS_FGUI_BEGIN

class GObject;
class GController;

class FGUI_IMPEXP GearBase
{
public:
    GearBase(GObject* owner);
    virtual ~GearBase();

    GController* getController() const { return _controller; }
    void setController(GController* value);

    virtual void updateFromRelations(float dx, float dy);
    virtual void apply();
    virtual void updateState();

    void setup(TXMLElement * xml);

    static bool disableAllTweenEffect;
    bool tween;
    tweenfunc::TweenType easeType;
    float tweenTime;
    float delay;

protected:
    virtual void addStatus(const std::string&  pageId, const std::string& value);
    virtual void init();

    GObject* _owner;
    GController* _controller;
    UINT32 _displayLockToken;
};

NS_FGUI_END

#endif
