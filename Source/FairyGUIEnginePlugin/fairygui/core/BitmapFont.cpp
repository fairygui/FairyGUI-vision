#include "BitmapFont.h"
#include "TextField.h"

NS_FGUI_BEGIN

BitmapFont::BitmapFont(const std::string & name) :BaseFont(name)
{
}

BitmapFont::~BitmapFont()
{
}

int BitmapFont::getCharacterIndexAtPos(const char * szText, const TextFormat & format, float fHorizPos, int iCharCount)
{
    if (iCharCount == -1)
        iCharCount = strlen(szText);
    if (iCharCount == 0)
        return 0;

    if (scaleEnabled)
        fHorizPos *= size / format.size;

    float x = 0;
    for (int i = 0; i < iCharCount; i++)
    {
        auto def = chars.find(szText[i]);
        if (def == chars.cend())
            continue;

        if (x >= fHorizPos)
            return i;

        if (x != 0)
            x += format.letterSpacing;
        x += def->second.advance;
    }
    return iCharCount;
}

bool BitmapFont::getTextDimension(const char * szText, const TextFormat & format, VRectanglef & dest, int iCharCount)
{
    if (iCharCount == -1)
        iCharCount = strlen(szText);
    if (iCharCount == 0)
        return true;

    float w = 0;
    float h = 0;
    for (int i = 0; i < iCharCount; i++)
    {
        char c = szText[i];
        if (c == ' ')
        {
            if (w != 0)
                w += format.letterSpacing;
            w += size / 2;
            h = MAX(size, h);
        }
        else
        {
            auto def = chars.find(c);
            if (def == chars.cend())
                continue;

            if (w != 0)
                w += format.letterSpacing;
            w += def->second.advance;
            h = MAX(def->second.lineHeight, h);
        }
    }

    if (scaleEnabled)
    {
        float scale = format.size / size;
        w *= scale;
        h *= scale;
    }
    dest.Set(0, 0, w, h);

    return true;
}

int BitmapFont::getLineHeight(const TextFormat & format)
{
    if (scaleEnabled)
    {
        float scale = format.size / size;
        return lineHeight * scale;
    }
    else
        return lineHeight;
}

void BitmapFont::prepareGraphics(NGraphics & graphics, TextRenderElement & re)
{
    hkvStringView view(re.text.c_str());
    float x = 0;
    while(view.IsValid())
    {
        hkUint32 c = view.GetCharacter();
        ++view;

        VColorRef color = colorEnabled ? re.format->color : V_RGBA_WHITE;

        if (c == ' ')
        {
            if (x != 0)
                x += re.format->letterSpacing;
            x += size / 2;
        }
        else
        {
            auto def = chars.find(c);
            if (def == chars.cend())
                continue;

            const BMGlyph& bg = def->second;
            if (x != 0)
                x += re.format->letterSpacing;

            VRectanglef rect;
            rect.m_vMin.set(re.pos.x + x + bg.offsetX, re.pos.y + bg.offsetY);
            rect.m_vMax.set(rect.m_vMin.x + bg.width, rect.m_vMin.y + bg.height);
            graphics.addQuad(rect, bg.uv, color);

            x += bg.advance;
        }
    }
}

NS_FGUI_END

