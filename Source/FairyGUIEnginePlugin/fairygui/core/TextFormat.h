#ifndef __TEXTFORMAT_H__
#define __TEXTFORMAT_H__

#include "FGUIMacros.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP TextFormat
{
public:
    TextFormat();
    TextFormat(const TextFormat & other);
    TextFormat &operator =(const TextFormat & other);

    void enableEffect(int effectFlag) { effect |= effectFlag; }
    void disableEffect(int effectFlag) { effect &= ~effectFlag; }
    bool hasEffect(int effectFlag) const { return (effect & effectFlag) != 0; }

    static const int OUTLINE = 1;
    static const int SHADOW = 2;
    static const int GLOW = 4;

    std::string font;
    float size;
	VColorRef color;
    bool bold;
    bool italics;
    bool underline;
    int lineSpacing;
    int letterSpacing;
	AlignType align;
    VertAlignType verticalAlign;

    int effect;
    VColorRef outlineColor;
    int outlineSize;
    VColorRef shadowColor;
    hkvVec2 shadowOffset;
    int shadowBlurRadius;
    VColorRef glowColor;

private:
    bool _hasColor;

    friend class MyXmlVisitor;
};

NS_FGUI_END

#endif
