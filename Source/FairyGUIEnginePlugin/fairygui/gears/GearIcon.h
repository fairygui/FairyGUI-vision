#ifndef __GEARICON_H__
#define __GEARICON_H__

#include "FGUIMacros.h"

#include "GearBase.h"

NS_FGUI_BEGIN

class GObject;

class FGUI_IMPEXP GearIcon : public GearBase
{
public:
    GearIcon(GObject* owner);
    virtual ~GearIcon();

    void apply() override;
    void updateState() override;

protected:
    void addStatus(const std::string&  pageId, const std::string& value) override;
    void init() override;

private:
    std::unordered_map<std::string, std::string> _storage;
    std::string _default;
};

NS_FGUI_END

#endif
