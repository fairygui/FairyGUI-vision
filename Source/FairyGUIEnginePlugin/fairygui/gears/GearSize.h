#ifndef __GEARSIZE_H__
#define __GEARSIZE_H__

#include "FGUIMacros.h"

#include "GearBase.h"

NS_FGUI_BEGIN

class GObject;

class FGUI_IMPEXP GearSize : public GearBase
{
public:
    GearSize(GObject* owner);
    virtual ~GearSize();

    void apply() override;
    void updateState() override;
    void updateFromRelations(float dx, float dy) override;

protected:
    void addStatus(const std::string&  pageId, const std::string& value) override;
    void init() override;

private:
    void onTweenUpdate(const hkvVec4& v, bool a, bool b);
    void onTweenComplete();

    std::unordered_map<std::string, hkvVec4> _storage;
    hkvVec4 _default;
    hkvVec4 _tweenTarget;
};

NS_FGUI_END

#endif
