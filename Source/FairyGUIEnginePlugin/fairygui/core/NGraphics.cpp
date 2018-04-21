#include "NGraphics.h"
#include "RenderContext.h"
#include "FGUIManager.h"
#include "TextField.h"
#include "NativeFont.h"

NS_FGUI_BEGIN

const VRectanglef NGraphics::FULL_UV = VRectanglef(0, 0, 1, 1);

NGraphics::NGraphics() :
    _texture(nullptr),
    _alpha(1),
    _dirty(true),
    _ignoreClipping(false),
    _matrixVersion(0),
    _enabled(true),
    _font(nullptr),
    _textElements(nullptr)
{
}

NGraphics::~NGraphics()
{
}

void NGraphics::setTexture(NTexture* value)
{
    _texture = value;
}

void NGraphics::setWhiteTexture()
{
    setTexture(FGUIManager::GlobalManager().getWhiteTexture());
}

void NGraphics::render(RenderContext* context, const hkvMat4& localToWorldMatrix, hkUint32 matrixVersion, float alpha)
{
    if (!_enabled)
        return;

    alpha *= context->alpha;

    if (_texture != nullptr && !_vertexBuffer.IsEmpty())
    {
        if (_dirty)
        {
            _dirty = false;
            _alpha = alpha;
            _matrixVersion = matrixVersion;

            int cnt = _vertexBuffer.GetSize();
            _alphaBackup.SetSize(cnt);
            _positionBackup.SetSize(cnt);
            for (int i = 0; i < cnt; i++)
            {
                Overlay2DVertex_t& m = _vertexBuffer[i];
                _alphaBackup[i] = m.color.a;
                _positionBackup[i] = m.screenPos;

                m.color.a = (UBYTE)(m.color.a * _alpha);
                m.screenPos = localToWorldMatrix.transformPosition(m.screenPos.getAsVec3(0)).getAsVec2();
            }
        }
        else
        {
            if (_alpha != alpha)
            {
                _alpha = alpha;

                int cnt = _vertexBuffer.GetSize();
                for (int i = 0; i < cnt; i++)
                {
                    Overlay2DVertex_t& m = _vertexBuffer[i];
                    m.color.a = (UBYTE)(_alphaBackup[i] * _alpha);
                }
            }

            if (_matrixVersion != matrixVersion)
            {
                _matrixVersion = matrixVersion;

                int cnt = _vertexBuffer.GetSize();
                for (int i = 0; i < cnt; i++)
                {
                    Overlay2DVertex_t& m = _vertexBuffer[i];
                    m.screenPos = localToWorldMatrix.transformPosition(_positionBackup[i].getAsVec3(0)).getAsVec2();
                }
            }
        }

        if (_ignoreClipping)
            context->enableClipping(false);

        context->getRenderer()->Draw2DBuffer(_vertexBuffer.GetSize(), _vertexBuffer.GetDataPointer(), _texture->getNativeTexture(), context->getRenderState());

        if (_ignoreClipping)
            context->enableClipping(true);
    }

    if (_textElements)
    {
        for (auto &re : *_textElements)
        {
            if (re->charCount == 0)
                continue;

            hkvVec2 screenPos = localToWorldMatrix.transformPosition(re->pos.getAsVec3(0)).getAsVec2();
            VColorRef color = _font->colorEnabled ? re->format->color : V_RGBA_WHITE;
            color.a *= alpha;

            hkvVec2 vDir(1, 0), vUp(0, -1);
            /*if (rotation != 0)
            {
                vDir.set(hkvMath::cosDeg(rotation), hkvMath::sinDeg(rotation));
                vUp.set(vDir.y, -vDir.x); // orthogonal
            }*/

            hkvVec3 scaling = localToWorldMatrix.getScalingFactors();
            if (_font->scaleEnabled)
                scaling *= re->format->size / _font->getFontSize();
            vDir.x *= scaling.x;
            vUp.y *= scaling.y;

            if (re->format->italics)
                vUp.x += 0.25f;

            if (_font->simulateOutline && re->format->hasEffect(TextFormat::OUTLINE))
            {
                _font->getVisFont()->PrintText(context->getRenderer(), screenPos + hkvVec2(-re->format->outlineSize, 0), vDir, vUp, re->text.c_str(), re->format->outlineColor, context->getRenderState());
                _font->getVisFont()->PrintText(context->getRenderer(), screenPos + hkvVec2(re->format->outlineSize, 0), vDir, vUp, re->text.c_str(), re->format->outlineColor, context->getRenderState());
                _font->getVisFont()->PrintText(context->getRenderer(), screenPos + hkvVec2(0, re->format->outlineSize), vDir, vUp, re->text.c_str(), re->format->outlineColor, context->getRenderState());
                _font->getVisFont()->PrintText(context->getRenderer(), screenPos + hkvVec2(0, -re->format->outlineSize), vDir, vUp, re->text.c_str(), re->format->outlineColor, context->getRenderState());
            }

            if (re->format->hasEffect(TextFormat::SHADOW))
                _font->getVisFont()->PrintText(context->getRenderer(), screenPos + re->format->shadowOffset, vDir, vUp, re->text.c_str(), re->format->shadowColor, context->getRenderState());

            _font->getVisFont()->PrintText(context->getRenderer(), screenPos, vDir, vUp, re->text.c_str(), color, context->getRenderState());
        }
    }
}

void NGraphics::clearMesh()
{
    _vertexBuffer.Clear();
    _alphaBackup.Clear();
    _positionBackup.Clear();
    _font = nullptr;
    _textElements = nullptr;
    _matrixVersion = 0;
    _alpha = 1;
}

void NGraphics::addVertex(const hkvVec2& pos, const hkvVec2& uv, const VColorRef& color)
{
    Overlay2DVertex_t v0;
    v0.Set(pos.x, pos.y, uv.x, uv.y, color);
    _vertexBuffer.PushBack(v0);
    _dirty = true;
}

void NGraphics::addQuad(const VRectanglef& drawRect, const VRectanglef& uvRect, const VColorRef& color)
{
    Overlay2DVertex_t v0;
    v0.Set(drawRect.m_vMin.x, drawRect.m_vMin.y, uvRect.m_vMin.x, uvRect.m_vMin.y, color);
    Overlay2DVertex_t v1;
    v1.Set(drawRect.m_vMax.x, drawRect.m_vMin.y, uvRect.m_vMax.x, uvRect.m_vMin.y, color);
    Overlay2DVertex_t v2;
    v2.Set(drawRect.m_vMax.x, drawRect.m_vMax.y, uvRect.m_vMax.x, uvRect.m_vMax.y, color);
    Overlay2DVertex_t v3;
    v3.Set(drawRect.m_vMin.x, drawRect.m_vMax.y, uvRect.m_vMin.x, uvRect.m_vMax.y, color);

    _vertexBuffer.PushBack(v0);
    _vertexBuffer.PushBack(v3);
    _vertexBuffer.PushBack(v1);
    _vertexBuffer.PushBack(v3);
    _vertexBuffer.PushBack(v2);
    _vertexBuffer.PushBack(v1);
    _dirty = true;
}

void NGraphics::drawRect(const VRectanglef& vertRect, float lineSize, const VColorRef& lineColor, const VColorRef& fillColor)
{
    if (lineSize == 0)
    {
        addQuad(vertRect, FULL_UV, fillColor);
    }
    else
    {
        VRectanglef rect;

        //left,right
        rect = VRectanglef(0, 0, lineSize, vertRect.GetSizeY());
        addQuad(rect, FULL_UV, lineColor);
        rect = VRectanglef(vertRect.GetSizeX() - lineSize, 0, vertRect.GetSizeX(), vertRect.GetSizeY());
        addQuad(rect, FULL_UV, lineColor);

        //top, bottom
        rect = VRectanglef(lineSize, 0, vertRect.GetSizeX() - lineSize, lineSize);
        addQuad(rect, FULL_UV, lineColor);
        rect = VRectanglef(lineSize, vertRect.GetSizeY() - lineSize, vertRect.GetSizeX() - lineSize, vertRect.GetSizeY());
        addQuad(rect, FULL_UV, lineColor);

        //middle
        rect = VRectanglef(lineSize, lineSize, vertRect.GetSizeX() - lineSize, vertRect.GetSizeY() - lineSize);
        addQuad(rect, FULL_UV, fillColor);
    }
}

void NGraphics::drawEllipse(const VRectanglef& vertRect, const VColorRef& color)
{
    float radiusX = vertRect.GetSizeX() / 2;
    float radiusY = vertRect.GetSizeY() / 2;
    int numSides = (int)hkvMath::ceil(hkvMath::pi() * (radiusX + radiusY) / 4);
    if (numSides < 6) numSides = 6;

    float angleDelta = 2 * hkvMath::pi() / numSides;
    float angle = 0;

    Overlay2DVertex_t v0;
    Overlay2DVertex_t v1;
    Overlay2DVertex_t v2;

    v0.Set(radiusX, radiusY, 0, 0, color);
    for (int i = 0; i <= numSides; i++)
    {
        if (i == numSides)
            angle = 0;

        v1.Set(hkvMath::cosRad(angle) * radiusX + radiusX, hkvMath::sinRad(angle) * radiusY + radiusY, 1, 1, color);
        if (i != 0)
        {
            _vertexBuffer.PushBack(v0);
            _vertexBuffer.PushBack(v1);
            _vertexBuffer.PushBack(v2);
        }

        v2 = v1;
        angle += angleDelta;
    }

    _dirty = true;
}

void NGraphics::drawText(NativeFont* font, std::vector<TextRenderElement*>* renderElements)
{
    _font = font;
    if (_font)
        _textElements = renderElements;
    else
        _textElements = nullptr;
}

void NGraphics::rotateUV(const VRectanglef& baseUVRect)
{
    float xMin = MIN(baseUVRect.m_vMin.x, baseUVRect.m_vMax.x);
    float yMin = baseUVRect.m_vMin.y;
    float yMax = baseUVRect.m_vMax.y;
    if (yMin > yMax)
    {
        yMin = yMax;
        yMax = baseUVRect.m_vMin.y;
    }

    float tmp;
    int cnt = _vertexBuffer.GetSize();
    for (int i = 0; i < cnt; i++)
    {
        Overlay2DVertex_t& m = _vertexBuffer[i];
        tmp = m.texCoord.y;
        m.texCoord.y = yMin + m.texCoord.x - xMin;
        m.texCoord.x = xMin + yMax - tmp;
    }
}

void NGraphics::tint(const VColorRef & color)
{
    int cnt = _vertexBuffer.GetSize();
    for (int i = 0; i < cnt; i++)
    {
        Overlay2DVertex_t& m = _vertexBuffer[i];
        m.color = color;
        _alphaBackup[i] = m.color.a;
        m.color.a = (UBYTE)(m.color.a * _alpha);
    }
}


NS_FGUI_END