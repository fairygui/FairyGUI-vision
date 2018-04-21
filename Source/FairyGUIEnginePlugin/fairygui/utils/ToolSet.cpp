#include <vector>
#include <sstream>

#include "ToolSet.h"

NS_FGUI_BEGIN

static std::vector<std::string> helperArray;

void ToolSet::splitString(const std::string &s, char delim, std::vector<std::string> &elems)
{
    elems.clear();
    if (s.empty())
        return;

    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    if (s.back() == delim)
        elems.push_back("");
}

void ToolSet::splitString(const std::string &s, char delim, hkvVec2& value, bool intType)
{
    splitString(s, delim, helperArray);
    if (intType)
    {
        value.x = (float)atoi(helperArray[0].c_str());
        if (helperArray.size() > 1)
            value.y = (float)atoi(helperArray[1].c_str());
        else
            value.y = value.x;
    }
    else
    {
        value.x = (float)atof(helperArray[0].c_str());
        if (helperArray.size() > 1)
            value.y = (float)atof(helperArray[1].c_str());
        else
            value.y = value.x;
    }
}

void ToolSet::splitString(const std::string &s, char delim, hkvVec4& value, bool intType)
{
    splitString(s, delim, helperArray);
    if (intType)
    {
        value.x = (float)atoi(helperArray[0].c_str());
        if (helperArray.size() > 1)
        {
            value.y = (float)atoi(helperArray[1].c_str());
            if (helperArray.size() > 2)
            {
                value.z = (float)atoi(helperArray[2].c_str());
                value.w = (float)atoi(helperArray[3].c_str());
            }
        }
        else
            value.y = value.z = value.w = value.x;
    }
    else
    {
        value.x = (float)atof(helperArray[0].c_str());
        if (helperArray.size() > 1)
        {
            value.y = (float)atof(helperArray[1].c_str());
            if (helperArray.size() > 2)
            {
                value.z = (float)atof(helperArray[2].c_str());
                value.w = (float)atof(helperArray[3].c_str());
            }
        }
        else
            value.y = value.z = value.w = value.x;
    }
}

void ToolSet::splitString(const std::string & s, char delim, std::string & str1, std::string& str2)
{
    splitString(s, delim, helperArray);
    str1 = helperArray[0];
    if (helperArray.size() > 1)
        str2 = helperArray[1];
    else
        str2 = "";
}

int ToolSet::findInStringArray(const std::vector<std::string>& arr, const std::string& str)
{
    auto iter = std::find(arr.cbegin(), arr.cend(), str);
    if (iter != arr.cend())
        return (int)(iter - arr.cbegin());

    return -1;
}

VColorRef ToolSet::convertFromHtmlColor(const char* str)
{
    size_t len = strlen(str);
    if (len < 7 || str[0] != '#')
        return V_RGBA_BLACK;

    VColorRef color;
    if (len == 9)
    {
        unsigned long v = strtoul(str + 1, NULL, 16);
        color.a = (v & 0xFF000000) >> 24;
        color.r = (v & 0x00FF0000) >> 16;
        color.g = (v & 0x0000FF00) >> 8;
        color.b = v & 0x000000FF;
    }
    else
    {
        unsigned long v = strtoul(str + 1, NULL, 16);
        color.a = 255;
        color.r = (v & 0xFF0000) >> 16;
        color.g = (v & 0x00FF00) >> 8;
        color.b = v & 0x0000FF;
    }

    return color;
}

VRectanglef ToolSet::transformRect(const VRectanglef& rect, const hkvMat4& localToWorld, const hkvMat4& worldToLocal)
{
    hkvVec3 points[4];
    points[0] = rect.m_vMin.getAsVec3(0);
    points[1] = hkvVec3(rect.m_vMax.x, rect.m_vMin.y, 0);
    points[2] = hkvVec3(rect.m_vMin.x, rect.m_vMax.y, 0);
    points[3] = rect.m_vMax.getAsVec3(0);
    localToWorld.transformPositions(points, 4);
    worldToLocal.transformPositions(points, 4);

    VRectanglef ret(HKVMATH_FLOAT_MAX_POS, HKVMATH_FLOAT_MAX_POS, HKVMATH_FLOAT_MAX_NEG, HKVMATH_FLOAT_MAX_NEG);
    for (int i = 0; i < 4; i++)
    {
        hkvVec3 v = points[i];

        if (v.x < ret.m_vMin.x) ret.m_vMin.x = v.x;
        if (v.x > ret.m_vMax.x) ret.m_vMax.x = v.x;
        if (v.y < ret.m_vMin.y) ret.m_vMin.y = v.y;
        if (v.y > ret.m_vMax.y) ret.m_vMax.y = v.y;
    }

    return ret;
}

VRectanglef ToolSet::unionRect(const VRectanglef & rect1, const VRectanglef & rect2)
{
    if (rect2.GetSizeX() == 0 || rect2.GetSizeY() == 0)
        return rect1;

    if (rect1.GetSizeX() == 0 || rect1.GetSizeX() == 0)
        return rect2;

    return VRectanglef(MIN(rect1.m_vMin.x, rect2.m_vMin.x), MIN(rect1.m_vMin.y, rect2.m_vMin.y),
        MAX(rect1.m_vMax.x, rect2.m_vMax.x), MAX(rect1.m_vMax.y, rect2.m_vMax.y));
}

VRectanglef ToolSet::intersection(const VRectanglef & rect1, const VRectanglef & rect2)
{
    if (rect1.GetSizeX() == 0 || rect1.GetSizeY() == 0 || rect2.GetSizeX() == 0 || rect2.GetSizeY() == 0)
        return VRectanglef(0, 0, 0, 0);

    float left = rect1.m_vMin.x > rect2.m_vMin.x ? rect1.m_vMin.x : rect2.m_vMin.x;
    float right = rect1.m_vMax.x < rect2.m_vMax.x ? rect1.m_vMax.x : rect2.m_vMax.x;
    float top = rect1.m_vMin.y > rect2.m_vMin.y ? rect1.m_vMin.y : rect2.m_vMin.y;
    float bottom = rect1.m_vMax.y < rect2.m_vMax.y ? rect1.m_vMax.y : rect2.m_vMax.y;

    if (left > right || top > bottom)
        return VRectanglef(0, 0, 0, 0);
    else
        return VRectanglef(left, top, right, bottom);
}

void ToolSet::flipRect(VRectanglef& rect, FlipType flip)
{
    if (flip == FlipType::HORIZONTAL || flip == FlipType::BOTH)
    {
        float tmp = rect.m_vMin.x;
        rect.m_vMin.x = rect.m_vMax.x;
        rect.m_vMax.x = tmp;
    }
    if (flip == FlipType::VERTICAL || flip == FlipType::BOTH)
    {
        float tmp = rect.m_vMin.y;
        rect.m_vMin.y = rect.m_vMax.y;
        rect.m_vMax.y = tmp;
    }
}

void ToolSet::flipInnerRect(float sourceWidth, float sourceHeight, VRectanglef& rect, FlipType flip)
{
    if (flip == FlipType::HORIZONTAL || flip == FlipType::BOTH)
    {
        float tmp = rect.GetSizeX();
        rect.m_vMin.x = sourceWidth - rect.m_vMax.x;
        rect.m_vMax.x = rect.m_vMin.x + tmp;
    }

    if (flip == FlipType::VERTICAL || flip == FlipType::BOTH)
    {
        float tmp = rect.GetSizeY();
        rect.m_vMin.y = sourceHeight - rect.m_vMax.y;
        rect.m_vMax.y = rect.m_vMin.y + tmp;
    }
}

PackageItemType ToolSet::parsePackageItemType(const char* p)
{
    if (!p)
        return PackageItemType::MISC;

    if (strcmp(p, "image") == 0)
        return PackageItemType::IMAGE;
    else if (strcmp(p, "movieclip") == 0)
        return PackageItemType::MOVIECLIP;
    else if (strcmp(p, "component") == 0)
        return PackageItemType::COMPONENT;
    else if (strcmp(p, "atlas") == 0)
        return PackageItemType::ATLAS;
    else if (strcmp(p, "sound") == 0)
        return PackageItemType::SOUND;
    else if (strcmp(p, "font") == 0)
        return PackageItemType::FONT;
    else if (strcmp(p, "misc") == 0)
        return PackageItemType::MISC;
    else
        return PackageItemType::MISC;
}

AlignType ToolSet::parseAlign(const char *p)
{
    if (!p)
        return AlignType::LEFT;

    if (strcmp(p, "left") == 0)
        return AlignType::LEFT;
    else if (strcmp(p, "center") == 0)
        return AlignType::CENTER;
    else if (strcmp(p, "right") == 0)
        return AlignType::RIGHT;
    else
        return AlignType::LEFT;
}

VertAlignType ToolSet::parseVerticalAlign(const char * p)
{
    if (!p)
        return VertAlignType::TOP;

    if (strcmp(p, "top") == 0)
        return VertAlignType::TOP;
    else if (strcmp(p, "middle") == 0)
        return VertAlignType::MIDDLE;
    else if (strcmp(p, "bottom") == 0)
        return VertAlignType::BOTTOM;
    else
        return VertAlignType::TOP;
}

int ToolSet::parseGearIndex(const char* p)
{
    if (!p || *p != 'g')
        return -1;

    if (strcmp(p, "gearDisplay") == 0)
        return 0;
    else if (strcmp(p, "gearXY") == 0)
        return 1;
    else if (strcmp(p, "gearSize") == 0)
        return 2;
    else if (strcmp(p, "gearLook") == 0)
        return 3;
    else if (strcmp(p, "gearColor") == 0)
        return 4;
    else if (strcmp(p, "gearAni") == 0)
        return 5;
    else if (strcmp(p, "gearText") == 0)
        return 6;
    else if (strcmp(p, "gearIcon") == 0)
        return 7;
    else
        return -1;
}

LoaderFillType ToolSet::parseFillType(const char * p)
{
    if (!p)
        return LoaderFillType::NONE;

    if (strcmp(p, "none") == 0)
        return LoaderFillType::NONE;
    else if (strcmp(p, "scale") == 0)
        return LoaderFillType::SCALE;
    else if (strcmp(p, "scaleMatchHeight") == 0)
        return LoaderFillType::SCALE_MATCH_HEIGHT;
    else if (strcmp(p, "scaleMatchWidth") == 0)
        return LoaderFillType::SCALE_MATCH_WIDTH;
    else if (strcmp(p, "scaleFree") == 0)
        return LoaderFillType::SCALE_FREE;
    else if (strcmp(p, "scaleNoBorder") == 0)
        return LoaderFillType::SCALE_NO_BORDER;
    else
        return LoaderFillType::NONE;
}

ButtonMode ToolSet::parseButtonMode(const char * p)
{
    if (!p)
        return ButtonMode::COMMON;

    if (strcmp(p, "Check") == 0)
        return ButtonMode::CHECK;
    else if (strcmp(p, "Radio") == 0)
        return ButtonMode::RADIO;
    else
        return ButtonMode::COMMON;
}

OverflowType ToolSet::parseOverflowType(const char * p)
{
    if (!p)
        return OverflowType::VISIBLE;

    if (strcmp(p, "visible") == 0)
        return OverflowType::VISIBLE;
    else if (strcmp(p, "hidden") == 0)
        return OverflowType::HIDDEN;
    else if (strcmp(p, "scroll") == 0)
        return OverflowType::SCROLL;
    else
        return OverflowType::VISIBLE;
}

ScrollType ToolSet::parseScrollType(const char * p)
{
    if (!p)
        return ScrollType::HORIZONTAL;

    if (strcmp(p, "horizontal") == 0)
        return ScrollType::HORIZONTAL;
    else if (strcmp(p, "vertical") == 0)
        return ScrollType::VERTICAL;
    else if (strcmp(p, "both") == 0)
        return ScrollType::BOTH;
    else
        return ScrollType::HORIZONTAL;
}

ScrollBarDisplayType ToolSet::parseScrollBarDisplayType(const char * p)
{
    if (!p)
        return ScrollBarDisplayType::DEFAULT;

    if (strcmp(p, "default") == 0)
        return ScrollBarDisplayType::DEFAULT;
    else if (strcmp(p, "visible") == 0)
        return ScrollBarDisplayType::VISIBLE;
    else if (strcmp(p, "auto") == 0)
        return ScrollBarDisplayType::AUTO;
    else if (strcmp(p, "hidden") == 0)
        return ScrollBarDisplayType::HIDDEN;
    else
        return ScrollBarDisplayType::DEFAULT;
}

ProgressTitleType ToolSet::parseProgressTitleType(const char * p)
{
    if (!p)
        return ProgressTitleType::PERCENT;

    if (strcmp(p, "percent") == 0)
        return ProgressTitleType::PERCENT;
    else if (strcmp(p, "valueAndmax") == 0)
        return ProgressTitleType::VALUE_MAX;
    else if (strcmp(p, "value") == 0)
        return ProgressTitleType::VALUE;
    else if (strcmp(p, "max") == 0)
        return ProgressTitleType::MAX;
    else
        return ProgressTitleType::PERCENT;
}

ListLayoutType ToolSet::parseListLayoutType(const char * p)
{
    if (!p)
        return ListLayoutType::SINGLE_COLUMN;

    if (strcmp(p, "column") == 0)
        return ListLayoutType::SINGLE_COLUMN;
    else if (strcmp(p, "row") == 0)
        return ListLayoutType::SINGLE_ROW;
    else if (strcmp(p, "flow_hz") == 0)
        return ListLayoutType::FLOW_HORIZONTAL;
    else if (strcmp(p, "flow_vt") == 0)
        return ListLayoutType::FLOW_VERTICAL;
    else if (strcmp(p, "pagination") == 0)
        return ListLayoutType::PAGINATION;
    else
        return ListLayoutType::SINGLE_COLUMN;
}

ListSelectionMode ToolSet::parseListSelectionMode(const char * p)
{
    if (!p)
        return ListSelectionMode::SINGLE;

    if (strcmp(p, "single") == 0)
        return ListSelectionMode::SINGLE;
    else if (strcmp(p, "multiple") == 0)
        return ListSelectionMode::MULTIPLE;
    else if (strcmp(p, "multipleSingleClick") == 0)
        return ListSelectionMode::MULTIPLE_SINGLECLICK;
    else if (strcmp(p, "none") == 0)
        return ListSelectionMode::NONE;
    else
        return ListSelectionMode::SINGLE;
}

ChildrenRenderOrder ToolSet::parseChildrenRenderOrder(const char * p)
{
    if (!p)
        return ChildrenRenderOrder::ASCENT;

    if (strcmp(p, "ascent") == 0)
        return ChildrenRenderOrder::ASCENT;
    else if (strcmp(p, "descent") == 0)
        return ChildrenRenderOrder::DESCENT;
    else if (strcmp(p, "arch") == 0)
        return ChildrenRenderOrder::ARCH;
    else
        return ChildrenRenderOrder::ASCENT;
}

GroupLayoutType ToolSet::parseGroupLayoutType(const char * p)
{
    if (!p)
        return GroupLayoutType::NONE;

    if (strcmp(p, "hz") == 0)
        return GroupLayoutType::HORIZONTAL;
    else if (strcmp(p, "vt") == 0)
        return GroupLayoutType::VERTICAL;
    else
        return GroupLayoutType::NONE;
}

PopupDirection ToolSet::parsePopupDirection(const char * p)
{
    if (!p)
        return PopupDirection::AUTO;

    if (strcmp(p, "auto") == 0)
        return PopupDirection::AUTO;
    else if (strcmp(p, "up") == 0)
        return PopupDirection::UP;
    else if (strcmp(p, "down") == 0)
        return PopupDirection::DOWN;
    else
        return PopupDirection::AUTO;
}

TextAutoSize ToolSet::parseTextAutoSize(const char * p)
{
    if (!p)
        return TextAutoSize::NONE;

    if (strcmp(p, "none") == 0)
        return TextAutoSize::NONE;
    else if (strcmp(p, "both") == 0)
        return TextAutoSize::BOTH;
    else if (strcmp(p, "height") == 0)
        return TextAutoSize::HEIGHT;
    else if (strcmp(p, "shrink") == 0)
        return TextAutoSize::SHRINK;
    else
        return TextAutoSize::NONE;
}

FlipType ToolSet::parseFlipType(const char * p)
{
    if (!p)
        return FlipType::NONE;

    if (strcmp(p, "both") == 0)
        return FlipType::BOTH;
    else if (strcmp(p, "hz") == 0)
        return FlipType::HORIZONTAL;
    else if (strcmp(p, "vt") == 0)
        return FlipType::VERTICAL;
    else
        return FlipType::NONE;
}

TransitionActionType ToolSet::parseTransitionActionType(const char * p)
{
    if (!p)
        return TransitionActionType::Unknown;

    if (strcmp(p, "XY") == 0)
        return TransitionActionType::XY;
    else if (strcmp(p, "Size") == 0)
        return TransitionActionType::Size;
    else if (strcmp(p, "Scale") == 0)
        return TransitionActionType::Scale;
    else if (strcmp(p, "Pivot") == 0)
        return TransitionActionType::Pivot;
    else if (strcmp(p, "Alpha") == 0)
        return TransitionActionType::Alpha;
    else if (strcmp(p, "Rotation") == 0)
        return TransitionActionType::Rotation;
    else if (strcmp(p, "Color") == 0)
        return TransitionActionType::Color;
    else if (strcmp(p, "Animation") == 0)
        return TransitionActionType::Animation;
    else if (strcmp(p, "Visible") == 0)
        return TransitionActionType::Visible;
    else if (strcmp(p, "Sound") == 0)
        return TransitionActionType::Sound;
    else if (strcmp(p, "Transition") == 0)
        return TransitionActionType::Transition;
    else if (strcmp(p, "Shake") == 0)
        return TransitionActionType::Shake;
    else if (strcmp(p, "ColorFilter") == 0)
        return TransitionActionType::ColorFilter;
    else if (strcmp(p, "Skew") == 0)
        return TransitionActionType::Skew;
    else
        return TransitionActionType::Unknown;
}

tweenfunc::TweenType ToolSet::parseEaseType(const char * p)
{
    if (!p)
        return tweenfunc::Expo_EaseOut;

    if (strcmp(p, "Linear") == 0)
        return tweenfunc::Linear;

    else if (strcmp(p, "Elastic.In") == 0)
        return tweenfunc::Elastic_EaseIn;
    else if (strcmp(p, "Elastic.Out") == 0)
        return tweenfunc::Elastic_EaseOut;
    else if (strcmp(p, "Elastic.InOut") == 0)
        return tweenfunc::Elastic_EaseInOut;

    else if (strcmp(p, "Quad.In") == 0)
        return tweenfunc::Quad_EaseIn;
    else if (strcmp(p, "Quad.Out") == 0)
        return tweenfunc::Quad_EaseOut;
    else if (strcmp(p, "Quad.InOut") == 0)
        return tweenfunc::Quad_EaseInOut;

    else if (strcmp(p, "Cube.In") == 0)
        return tweenfunc::Cubic_EaseIn;
    else if (strcmp(p, "Cube.Out") == 0)
        return tweenfunc::Cubic_EaseOut;
    else if (strcmp(p, "Cube.InOut") == 0)
        return tweenfunc::Cubic_EaseInOut;

    else if (strcmp(p, "Quart.In") == 0)
        return tweenfunc::Quart_EaseIn;
    else if (strcmp(p, "Quart.Out") == 0)
        return tweenfunc::Quart_EaseOut;
    else if (strcmp(p, "Quart.InOut") == 0)
        return tweenfunc::Quart_EaseInOut;

    else if (strcmp(p, "Sine.In") == 0)
        return tweenfunc::Sine_EaseIn;
    else if (strcmp(p, "Sine.Out") == 0)
        return tweenfunc::Sine_EaseOut;
    else if (strcmp(p, "Sine.InOut") == 0)
        return tweenfunc::Sine_EaseInOut;

    else if (strcmp(p, "Bounce.In") == 0)
        return tweenfunc::Bounce_EaseIn;
    else if (strcmp(p, "Bounce.Out") == 0)
        return tweenfunc::Bounce_EaseOut;
    else if (strcmp(p, "Bounce.InOut") == 0)
        return tweenfunc::Bounce_EaseInOut;

    else if (strcmp(p, "Circ.In") == 0)
        return tweenfunc::Circ_EaseIn;
    else if (strcmp(p, "Circ.Out") == 0)
        return tweenfunc::Circ_EaseOut;
    else if (strcmp(p, "Circ.InOut") == 0)
        return tweenfunc::Circ_EaseInOut;

    else if (strcmp(p, "Expo.In") == 0)
        return tweenfunc::Expo_EaseIn;
    else if (strcmp(p, "Expo.Out") == 0)
        return tweenfunc::Expo_EaseOut;
    else if (strcmp(p, "Expo.InOut") == 0)
        return tweenfunc::Expo_EaseInOut;

    else if (strcmp(p, "Back.In") == 0)
        return tweenfunc::Back_EaseIn;
    else if (strcmp(p, "Back.Out") == 0)
        return tweenfunc::Back_EaseOut;
    else if (strcmp(p, "Back.InOut") == 0)
        return tweenfunc::Back_EaseInOut;

    else
        return tweenfunc::Expo_EaseOut;
}

FastSplitter::FastSplitter() :data(nullptr), dataLength(-1), delimiter('\0')
{
}

void FastSplitter::start(const char * data, ssize_t dataLength, char delimiter)
{
    this->data = data;
    this->dataLength = dataLength;
    this->delimiter = delimiter;
    this->textLength = -1;
}

bool FastSplitter::next()
{
    if (dataLength < 0)
        return false;

    if (dataLength == 0)
    {
        dataLength = -1;
        textLength = 0;
        return true;
    }

    data += textLength + 1;
    char* found = (char*)memchr(data, (int)delimiter, dataLength);
    if (found)
        textLength = found - data;
    else
        textLength = dataLength;
    dataLength -= (textLength + 1);

    return true;
}

const char * FastSplitter::getText()
{
    if (textLength > 0)
        return data;
    else
        return nullptr;
}

ssize_t FastSplitter::getTextLength()
{
    return textLength;
}

void FastSplitter::getKeyValuePair(char* keyBuf, ssize_t keyBufSize, char* valueBuf, ssize_t valueBufSize)
{
    if (textLength == 0)
    {
        keyBuf[0] = '\0';
        valueBuf[0] = '\0';
    }
    else
    {
        char* found = (char*)memchr(data, (int)'=', textLength);
        if (found)
        {
            ssize_t len = hkvMath::Min<ssize_t>(keyBufSize - 1, found - data);
            memcpy(keyBuf, data, len);
            keyBuf[len] = '\0';

            len = hkvMath::Min<ssize_t>(valueBufSize - 1, textLength - (found - data) - 1);
            memcpy(valueBuf, found + 1, len);
            valueBuf[len] = '\0';
        }
        else
        {
            ssize_t len = hkvMath::Min<ssize_t>(valueBufSize - 1, textLength);
            memcpy(keyBuf, data, len);
            keyBuf[len] = '\0';
            valueBuf[0] = '\0';
        }
    }
}

NS_FGUI_END
