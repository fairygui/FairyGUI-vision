#ifndef __FIELDTYPES_H__
#define __FIELDTYPES_H__

#include "FGUIMacros.h"

NS_FGUI_BEGIN

enum class PackageItemType
{
    IMAGE,
    MOVIECLIP,
    SOUND,
    COMPONENT,
    ATLAS,
    FONT,
    MISC
};

enum class AlignType
{
	LEFT,
	CENTER,
	RIGHT
};

enum class VertAlignType
{
	TOP,
	MIDDLE,
	BOTTOM
};

enum class ButtonMode
{
    COMMON,
    CHECK,
    RADIO
};

enum class ChildrenRenderOrder
{
    ASCENT,
    DESCENT,
    ARCH,
};

enum class OverflowType
{
    VISIBLE,
    HIDDEN,
    SCROLL
};

enum class ScrollType
{
    HORIZONTAL,
    VERTICAL,
    BOTH
};

enum ScrollBarDisplayType
{
    DEFAULT,
    VISIBLE,
    AUTO,
    HIDDEN
};

enum class LoaderFillType
{
    NONE,
    SCALE,
    SCALE_MATCH_HEIGHT,
    SCALE_MATCH_WIDTH,
    SCALE_FREE,
    SCALE_NO_BORDER
};

enum class ProgressTitleType
{
    PERCENT,
    VALUE_MAX,
    VALUE,
    MAX
};

enum class ListLayoutType
{
    SINGLE_COLUMN,
    SINGLE_ROW,
    FLOW_HORIZONTAL,
    FLOW_VERTICAL,
    PAGINATION
};

enum class ListSelectionMode
{
    SINGLE,
    MULTIPLE,
    MULTIPLE_SINGLECLICK,
    NONE
};

enum class GroupLayoutType
{
    NONE,
    HORIZONTAL,
    VERTICAL
};

enum class PopupDirection
{
    AUTO,
    UP,
    DOWN
};

enum class TextAutoSize
{
    NONE,
    BOTH,
    HEIGHT,
    SHRINK
};

enum class FlipType
{
    NONE,
    HORIZONTAL,
    VERTICAL,
    BOTH
};

enum class TransitionActionType
{
    XY,
    Size,
    Scale,
    Pivot,
    Alpha,
    Rotation,
    Color,
    Animation,
    Visible,
    Sound,
    Transition,
    Shake,
    ColorFilter,
    Skew,
    Unknown
};

enum class ScreenMatchMode
{
    MatchWidthOrHeight,
    MatchWidth,
    MatchHeight
};

NS_FGUI_END

#endif