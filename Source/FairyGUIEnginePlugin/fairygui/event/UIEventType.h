#ifndef __UIEVENTTYPE_H__
#define __UIEVENTTYPE_H__

#include "FGUIMacros.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP UIEventType
{
public:
    static const int AddedToStage = 0;
    static const int RemoveFromStage = 1;
    static const int Changed = 2;
    static const int Submit = 3;
    static const int FocusIn = 4;
    static const int FocusOut = 5;
    static const int StageTouchBegin = 9;

    static const int TouchBegin = 10;
    static const int TouchMove = 11;
    static const int TouchEnd = 12;
    static const int Click = 13;
    static const int RollOver = 14;
    static const int RollOut = 15;
    static const int MouseWheel = 16;
    static const int RightClick = 17;
    static const int MiddleClick = 18;

    static const int PositionChange = 20;
    static const int SizeChange = 21;

    static const int KeyDown = 30;
    static const int KeyUp = 31;

    static const int Scroll = 40;
    static const int ScrollEnd = 41;
    static const int PullDownRelease = 42;
    static const int PullUpRelease = 43;

    static const int ClickItem = 50;
    static const int ClickLink = 51;
    static const int ClickMenu = 52;
    static const int RightClickItem = 53;

    static const int DragStart = 60;
    static const int DragMove = 61;
    static const int DragEnd = 62;
    static const int Drop = 63;

    static const int GearStop = 70;
};

NS_FGUI_END

#endif
