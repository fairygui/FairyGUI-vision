#ifndef __BITMAP_FONT_H__
#define __BITMAP_FONT_H__

#include "FGUIMacros.h"
#include "BaseFont.h"

NS_FGUI_BEGIN

class TextRenderElement;

class FGUI_IMPEXP BitmapFont : public BaseFont
{
public:
    BitmapFont(const std::string& name);
    virtual ~BitmapFont();

    virtual int getCharacterIndexAtPos(const char *szText, const TextFormat & format, float fHorizPos, int iCharCount = -1) override;
    virtual bool getTextDimension(const char *szText, const TextFormat & format, VRectanglef &dest, int iCharCount = -1) override;
    virtual int getLineHeight(const TextFormat & format);
    virtual void prepareGraphics(NGraphics& graphics, TextRenderElement& re) override;

    void setTexture(NTexture* value) { _texture = value; }

    struct BMGlyph
    {
        int offsetX;
        int offsetY;
        int width;
        int height;
        int advance;
        int lineHeight;
        VRectanglef uv;
    };
    std::unordered_map<hkUint32, BMGlyph> chars;

    int size;
    int lineHeight;

protected:
    float scale;
};

NS_FGUI_END

#endif