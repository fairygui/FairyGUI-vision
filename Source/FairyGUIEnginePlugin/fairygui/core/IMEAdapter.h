#ifndef __IMEADAPTER_H__
#define __IMEADAPTER_H__

#include "FGUIMacros.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP IMEAdapter
{
public:
    enum class CompositionMode
    {
        Auto = 0,
        On = 1,
        Off = 2
    };

    static void setCursorPos(const hkvVec2& pos)
    {
    }

    static void setCompositionMode(CompositionMode mode)
    {
    }

    static std::string getCompositionString()
    {
        return "";
    }
};

NS_FGUI_END

#endif
