#ifndef __ACTION_UTILS_H__
#define __ACTION_UTILS_H__

#include "FGUIMacros.h"
#include "third_party/cc/CCActionManager.h"
#include "third_party/cc/CCActionInterval.h"
#include "third_party/cc/CCActionInstant.h"
#include "third_party/cc/CCActionEase.h"
#include "third_party/cc/CCActionCustom.h"

NS_FGUI_BEGIN

enum InternalActionTag
{
    GEAR_XY_ACTION = 0xCC2100,
    GEAR_SIZE_ACTION,
    GEAR_LOOK_ACTION,
    GEAR_COLOR_ACTION,
    PROGRESS_ACTION,
    TRANSITION_ACTION //remind:keep TRANSITION_ACTION as the last item
};

class FGUI_IMPEXP ActionUtils
{
public:
    static ActionInterval* createEaseAction(tweenfunc::TweenType tweenType, ActionInterval* action);
    static ActionInterval* composeActions(ActionInterval* mainAction, tweenfunc::TweenType easeType, float delay, std::function<void()> func, int tag = Action::INVALID_TAG);
};

NS_FGUI_END

#endif
