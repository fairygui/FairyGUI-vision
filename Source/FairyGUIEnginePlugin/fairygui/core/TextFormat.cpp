#include "TextFormat.h"

NS_FGUI_BEGIN

TextFormat::TextFormat() :
    size(12),
    color(V_RGBA_BLACK),
    bold(false),
    italics(false),
    underline(false),
    lineSpacing(3),
    letterSpacing(0),
    align(AlignType::LEFT),
    verticalAlign(VertAlignType::TOP),
    effect(0),
    outlineSize(1),
    outlineColor(0, 0, 0, 255),
    shadowBlurRadius(0),
    shadowOffset(0, 0),
    shadowColor(0, 0, 0, 255),
    glowColor(0, 0, 0, 255),
    _hasColor(false)
{
}

TextFormat::TextFormat(const TextFormat & other)
{
    *this = other;
}

TextFormat & TextFormat::operator=(const TextFormat & other)
{
    if (this != &other)
    {
        font = other.font;
        size = other.size;
        color = other.color;
        bold = other.bold;
        italics = other.italics;
        underline = other.underline;
        lineSpacing = other.lineSpacing;
        letterSpacing = other.letterSpacing;
        align = other.align;
        verticalAlign = other.verticalAlign;
        effect = other.effect;
        outlineColor = other.outlineColor;
        outlineSize = other.outlineSize;
        shadowColor = other.shadowColor;
        shadowOffset = other.shadowOffset;
        shadowBlurRadius = other.shadowBlurRadius;
        glowColor = other.glowColor;
        _hasColor = other._hasColor;
    }
    return *this;
}


NS_FGUI_END