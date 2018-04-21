#ifndef __GEARXY_H__
#define __GEARXY_H__

#include "FGUIMacros.h"

#include "GearBase.h"

NS_FGUI_BEGIN

class GObject;

class FGUI_IMPEXP GearXY : public GearBase
{
public:
    GearXY(GObject* owner);
    virtual ~GearXY();

    void apply() override;
    void updateState() override;
    void updateFromRelations(float dx, float dy) override;

protected:
    void addStatus(const std::string&  pageId, const std::string& value) override;
    void init() override;

private:
    void onTweenUpdate(const hkvVec2& v);
    void onTweenComplete();

    std::unordered_map<std::string, hkvVec2> _storage;
    hkvVec2 _default;
    hkvVec2 _tweenTarget;
    
};

NS_FGUI_END

#endif
