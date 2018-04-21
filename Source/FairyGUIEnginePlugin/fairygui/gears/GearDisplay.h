#ifndef __GEARDISPLAY_H__
#define __GEARDISPLAY_H__

#include "FGUIMacros.h"

#include "GearBase.h"

NS_FGUI_BEGIN

class GObject;

class FGUI_IMPEXP GearDisplay : public GearBase
{
public:
    GearDisplay(GObject* owner);
    virtual ~GearDisplay();

    void apply() override;
    void updateState() override;

    UINT32 addLock();
    void releaseLock(UINT32 token);
    bool isConnected();

    std::vector<std::string> pages;

protected:
    void addStatus(const std::string&  pageId, const std::string& value) override;
    void init() override;

private:
    int _visible;
};

NS_FGUI_END

#endif
