#ifndef __NTEXTURE_H__
#define __NTEXTURE_H__

#include "FGUIMacros.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP NTexture : public Ref
{
public:
    NTexture();
    NTexture(VTextureObject* texture, float xScale = 1.0f, float yScale = 1.0f);
    NTexture(NTexture* root, const VRectanglef& region, bool rotated = false);
    virtual ~NTexture();

    VTextureObject* getNativeTexture() const { return _nativeTexture; }
    const VRectanglef& getUVRect() const { return _uvRect; }
    const bool isRotated() const { return _rotated; }

    int getWidth() const { return _size.x; }
    int getHeight() const { return _size.y; }

    float lastActive;

private:
    void updateSize();

    VTextureObjectPtr _nativeTexture;
    VRectanglef _uvRect;
    VRectanglef* _region;
    hkvVec2 _size;
    bool _rotated;
};


typedef VSmartPtr<NTexture> NTexturePtr;

NS_FGUI_END

#endif