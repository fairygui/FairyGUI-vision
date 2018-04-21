#ifndef __NGRAPHICS_H__
#define __NGRAPHICS_H__

#include "FGUIMacros.h"
#include "NTexture.h"

NS_FGUI_BEGIN

class NTexture;
class RenderContext;
class TextRenderElement;
class NativeFont;

class FGUI_IMPEXP NGraphics
{
public:
    NGraphics();
    ~NGraphics();

    NTexture* getTexture() const { return _texture; }
    void setTexture(NTexture* value);
    void setWhiteTexture();
    void setIgnoreClipping(bool value) { _ignoreClipping = value; }

    bool isEnabled() const { return _enabled; }
    void setEnabled(bool value) { _enabled = value; }

    void addVertex(const hkvVec2& pos, const hkvVec2& uv, const VColorRef& color);
    void addQuad(const VRectanglef& drawRect, const VRectanglef& uvRect, const VColorRef& color);
    void drawRect(const VRectanglef& vertRect, float lineSize, const VColorRef& lineColor, const VColorRef& fillColor);
    void drawEllipse(const VRectanglef& vertRect, const VColorRef& color);
    void drawText(NativeFont* font, std::vector<TextRenderElement*>* renderElements);
    void clearMesh();
    void rotateUV(const VRectanglef& baseUVRect);
    void tint(const VColorRef& color);
    void blink() { _enabled = !_enabled; }

    void render(RenderContext* context, const hkvMat4& localToWorldMatrix, hkUint32 matrixVersion, float alpha);

    static const VRectanglef FULL_UV;

private:
    RefPtr<NTexture> _texture;
    hkvArray<Overlay2DVertex_t> _vertexBuffer;
    NativeFont* _font;
    std::vector<TextRenderElement*>* _textElements;
    hkvArray<UBYTE> _alphaBackup;
    hkvArray<hkvVec2> _positionBackup;
    float _alpha;
    bool _dirty;
    bool _ignoreClipping;
    bool _enabled;
    hkUint32 _matrixVersion;
};

NS_FGUI_END

#endif