#ifndef __GEARLOOK_H__
#define __GEARLOOK_H__

#include "FGUIMacros.h"

#include "GearBase.h"

NS_FGUI_BEGIN

class GObject;

class FGUI_IMPEXP GearLook : public GearBase
{
public:
    GearLook(GObject* owner);
    virtual ~GearLook();

    void apply() override;
    void updateState() override;

protected:
    void addStatus(const std::string&  pageId, const std::string& value) override;
    void init() override;

private:
    void onTweenUpdate(const hkvVec2& v, bool a, bool b);
    void onTweenComplete();

    class GearLookValue
    {
    public:
        float alpha;
        float rotation;
        bool grayed;
        bool touchable;

        GearLookValue();
        GearLookValue(float alpha, float rotation, bool grayed, bool touchable);
    };

    std::unordered_map<std::string, GearLookValue> _storage;
    GearLookValue _default;
    hkvVec2 _tweenTarget;
};

NS_FGUI_END

#endif
