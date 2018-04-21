#ifndef __TOOLSET_H__
#define __TOOLSET_H__

#include "FGUIMacros.h"
#include "third_party/cc/CCTweenFunction.h"

NS_FGUI_BEGIN

const VRectanglef EMPTY_RECT(0, 0, 0, 0);

class ToolSet
{
public:
    static void splitString(const std::string &s, char delim, std::vector<std::string> &elems);
    static void splitString(const std::string &s, char delim, hkvVec2& value, bool intType = false);
    static void splitString(const std::string &s, char delim, hkvVec4& value, bool intType = false);
    static void splitString(const std::string &s, char delim, std::string& str1, std::string& str2);
    static int findInStringArray(const std::vector<std::string>& arr, const std::string& str);

    static VColorRef convertFromHtmlColor(const char* str);
	static VRectanglef transformRect(const VRectanglef& rect, const hkvMat4& localToWorld, const hkvMat4& worldToLocal); 
    static VRectanglef unionRect(const VRectanglef& rect1, const VRectanglef& rect2);
    static VRectanglef intersection(const VRectanglef & rect1, const VRectanglef & rect2);
    static void flipRect(VRectanglef& rect, FlipType flip);
    static void flipInnerRect(float sourceWidth, float sourceHeight, VRectanglef& rect, FlipType flip);

    static PackageItemType parsePackageItemType(const char * p);
    static AlignType parseAlign(const char *p);
    static VertAlignType parseVerticalAlign(const char *p);
    static int parseGearIndex(const char* p);
    static LoaderFillType parseFillType(const char *p);
    static ButtonMode parseButtonMode(const char*p);
    static OverflowType parseOverflowType(const char*p);
    static ScrollType parseScrollType(const char*p);
    static ScrollBarDisplayType parseScrollBarDisplayType(const char*p);
    static ProgressTitleType parseProgressTitleType(const char*p);
    static ListLayoutType parseListLayoutType(const char*p);
    static ListSelectionMode parseListSelectionMode(const char*p);
   static ChildrenRenderOrder parseChildrenRenderOrder(const char*p);
    static GroupLayoutType parseGroupLayoutType(const char*p);
    static PopupDirection parsePopupDirection(const char*p);
    static TextAutoSize parseTextAutoSize(const char*p);
    static FlipType parseFlipType(const char*p);
    static TransitionActionType parseTransitionActionType(const char*p);
    static tweenfunc::TweenType parseEaseType(const char*p);
};

class FastSplitter
{
public:
    FastSplitter();
    void start(const char* data, ssize_t dataLength, char delimiter);
    bool next();
    const char* getText();
    ssize_t getTextLength();
    void getKeyValuePair(char* keyBuf, ssize_t keyBufSize, char* valueBuf, ssize_t valueBufSize);

private:
    const char* data;
    ssize_t dataLength;
    ssize_t textLength;
    char delimiter;
};

NS_FGUI_END

#endif
