#ifndef __UICONFIG_H__
#define __UICONFIG_H__

#include "FGUIMacros.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP UIConfig
{
public:
    static std::string defaultFont;
    static std::string buttonSound;
    static float buttonSoundVolumeScale;
    static int defaultScrollStep;
    static float defaultScrollDecelerationRate;
    static bool defaultScrollTouchEffect;
    static bool defaultScrollBounceEffect;
    static ScrollBarDisplayType defaultScrollBarDisplay;
    static std::string verticalScrollBar;
    static std::string horizontalScrollBar;
    static int touchDragSensitivity;
    static int clickDragSensitivity;
    static int touchScrollSensitivity;
    static int defaultComboBoxVisibleItemCount;
    static std::string globalModalWaiting;
    static VColorRef modalLayerColor;
    static std::string tooltipsWin;
    static bool bringWindowToFrontOnClick;
    static std::string windowModalWaiting;
    static std::string popupMenu;
    static std::string popupMenu_seperator;
    static float inputCaretSize;
    static VColorRef inputHighlightColor;

private:
};

NS_FGUI_END

#endif
