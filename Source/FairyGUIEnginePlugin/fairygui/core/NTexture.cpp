#include "NTexture.h"

NS_FGUI_BEGIN

NTexture::NTexture() :
    lastActive(0),
    _nativeTexture(nullptr),
    _region(nullptr),
    _rotated(false)
{
    updateSize();
}

NTexture::NTexture(VTextureObject * texture, float xScale, float yScale) :
    lastActive(0),
    _region(nullptr),
    _rotated(false)
{
    _nativeTexture = texture;
    _uvRect.Set(0, 0, xScale, yScale);
    updateSize();
}

NTexture::NTexture(NTexture * root, const VRectanglef & region, bool rotated) :
    lastActive(0)
{
    _nativeTexture = root->getNativeTexture();
    _region = new VRectanglef(region);
    if (_nativeTexture != nullptr)
        _uvRect.Set(region.m_vMin.x / _nativeTexture->GetTextureWidth(), region.m_vMin.y / _nativeTexture->GetTextureHeight(),
            region.m_vMax.x / _nativeTexture->GetTextureWidth(), region.m_vMax.y / _nativeTexture->GetTextureHeight());
    else
        _uvRect.Set(0, 0, 1, 1);
    _rotated = rotated;

    if (rotated)
    {
        float tmp = region.GetSizeX();
        _region->m_vMax.x = _region->m_vMin.x + _region->GetSizeY();
        _region->m_vMax.y = _region->m_vMin.y + tmp;

        tmp = _uvRect.GetSizeX();
        _uvRect.m_vMax.x = _uvRect.m_vMin.x + _uvRect.GetSizeY();
        _uvRect.m_vMax.y = _uvRect.m_vMin.y + tmp;
    }

    updateSize();
}

void NTexture::updateSize()
{
    if (_region != nullptr)
        _size = _region->GetSize();
    else if (_nativeTexture != nullptr)
        _size.set(_nativeTexture->GetTextureWidth(), _nativeTexture->GetTextureHeight());
    else
        _size.set(2, 2);
}

NTexture::~NTexture()
{
    CC_SAFE_DELETE(_region);
}

NS_FGUI_END