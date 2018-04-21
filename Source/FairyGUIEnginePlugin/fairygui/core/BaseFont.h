#ifndef __BASE_FONT_H__
#define __BASE_FONT_H__

#include "FGUIMacros.h"
#include "TextFormat.h"
#include "NGraphics.h"

NS_FGUI_BEGIN

class TextRenderElement;

class FGUI_IMPEXP BaseFont
{
public:
    BaseFont(const std::string& name) :scaleEnabled(true), colorEnabled(true), _texture(nullptr) { _fontName = name; }
    virtual ~BaseFont() { CC_SAFE_RELEASE(_texture); }

    virtual const std::string& getName() const { return _fontName; }
    virtual NTexture* getTexture() const { return _texture; }
    virtual int getCharacterIndexAtPos(const char *szText, const TextFormat & format, float fHorizPos, int iCharCount = -1) = 0;
    virtual bool getTextDimension(const char *szText, const TextFormat & format, VRectanglef &dest, int iCharCount = -1) = 0;
    virtual int getLineHeight(const TextFormat & format) = 0;
    virtual void prepareGraphics(NGraphics& graphics, TextRenderElement& renderElement) = 0;

    bool scaleEnabled;
    bool colorEnabled;

protected:
    std::string _fontName;
    NTexture* _texture;
};

NS_FGUI_END

#endif