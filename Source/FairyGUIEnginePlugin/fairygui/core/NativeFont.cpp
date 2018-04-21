#include "NativeFont.h"
#include "TextField.h"
#include "RenderContext.h"
#include "FGUIManager.h"

NS_FGUI_BEGIN

NativeFont::NativeFont(const std::string & name, const std::string & filePath, int fontSize, bool simulateOutline) :BaseFont(name)
{
    _visFontPtr = Vision::Fonts.LoadFont(filePath.c_str());
    if (_visFontPtr == nullptr)
        _visFontPtr = &Vision::Fonts.DebugFont();
    _texture = FGUIManager::GlobalManager().getWhiteTexture();
    _texture->retain();
    scaleEnabled = fontSize != 0;
    _fontSize = scaleEnabled ? fontSize : _visFontPtr->m_fLineHeight;
    this->simulateOutline = simulateOutline;
}

NativeFont::~NativeFont()
{
}

int NativeFont::getCharacterIndexAtPos(const char * szText, const TextFormat & format, float fHorizPos, int iCharCount)
{
    float scale = scaleEnabled ? _fontSize / format.size : 1;
    return _visFontPtr->GetCharacterIndexAtPos(szText, fHorizPos * scale, iCharCount, false);
}

bool NativeFont::getTextDimension(const char * szText, const TextFormat & format, VRectanglef & dest, int iCharCount)
{
    float scale = scaleEnabled ? format.size / _fontSize : 1;
    if (_visFontPtr->GetTextDimension(szText, dest, iCharCount))
    {
        dest.m_vMax *= scale;
        return true;
    }
    else
        return false;
}

int NativeFont::getLineHeight(const TextFormat & format)
{
    return scaleEnabled ? (format.size * _visFontPtr->m_fLineHeight / _fontSize) : _visFontPtr->m_fLineHeight;
}

void NativeFont::prepareGraphics(NGraphics & graphics, TextRenderElement & re)
{
    if (!re.format->underline || re.size.x == 0)
        return;

    graphics.addQuad(VRectanglef(re.pos.x, re.pos.y + re.size.y, re.pos.x + re.size.x, re.pos.y + re.size.y + 2), NGraphics::FULL_UV, re.format->color);

}

NS_FGUI_END

