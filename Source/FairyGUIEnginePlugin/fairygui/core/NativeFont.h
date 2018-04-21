#ifndef __NATIVE_FONT_H__
#define __NATIVE_FONT_H__

#include "FGUIMacros.h"
#include "BaseFont.h"

NS_FGUI_BEGIN

class RenderContext;
class TextRenderElement;

class FGUI_IMPEXP NativeFont : public BaseFont
{
public:
    NativeFont(const std::string& name, const std::string& filePath, int fontSize, bool simulateOutline);
    virtual ~NativeFont();

    virtual int getCharacterIndexAtPos(const char *szText, const TextFormat & format, float fHorizPos, int iCharCount = -1) override;
    virtual bool getTextDimension(const char *szText, const TextFormat & format, VRectanglef &dest, int iCharCount = -1) override;
    virtual int getLineHeight(const TextFormat & format)  override;
    virtual void prepareGraphics(NGraphics& graphics, TextRenderElement& re) override;

    VisFont_cl* getVisFont() const { return _visFontPtr; }
    float getFontSize() const { return _fontSize; }

    bool simulateOutline;
protected:
    VisFontPtr _visFontPtr;
    float _scale;
    float _fontSize;
};

NS_FGUI_END

#endif