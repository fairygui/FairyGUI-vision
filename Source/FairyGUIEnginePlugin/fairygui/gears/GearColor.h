#ifndef __GEARCOLOR_H__
#define __GEARCOLOR_H__

#include "FGUIMacros.h"

#include "GearBase.h"

NS_FGUI_BEGIN

class GObject;

class FGUI_IMPEXP IColorGear
{
public:
    virtual const VColorRef& getColor() const = 0;
    virtual void setColor(const VColorRef& value) = 0;

    virtual const VColorRef& getOutlineColor() const { return V_RGBA_BLACK; };
    virtual void setOutlineColor(const VColorRef& value) {};
};

class FGUI_IMPEXP GearColor : public GearBase
{
public:
    GearColor(GObject* owner);
    virtual ~GearColor();

    void apply() override;
    void updateState() override;

protected:
    void addStatus(const std::string&  pageId, const std::string& value) override;
    void init() override;

private:
    void onTweenUpdate(const hkvVec4& v);
    void onTweenComplete();

    class GearColorValue
    {
    public:
        VColorRef color;
        VColorRef outlineColor;

        GearColorValue();
        GearColorValue(const VColorRef& color, const VColorRef& outlineColor);
    };

    std::unordered_map<std::string, GearColorValue> _storage;
    GearColorValue _default;
    hkvVec4 _tweenTarget;
};

NS_FGUI_END

#endif
