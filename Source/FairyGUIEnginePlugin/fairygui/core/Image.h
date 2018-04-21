#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "FGUIMacros.h"
#include "DisplayObject.h"
#include "NTexture.h"

NS_FGUI_BEGIN

class NTexture;

class FGUI_IMPEXP Image : public DisplayObject
{
public:
    CREATE_FUNC(Image);

    NTexture* getTexture() const { return _graphics->getTexture(); }
    void setTexture(NTexture* value);
    void setNativeSize();

    const VColorRef& getColor() const { return _color; }
    void setColor(const VColorRef& value);

    FlipType getFlip() const { return _flip; }
    void setFlip(FlipType value);

    const VRectanglef& getScale9Grid() const;
    void setScale9Grid(const VRectanglef& value);

    bool isScaleByTile() const { return _scaleByTile; }
    void setScaleByTile(bool value);

    int getTileGridIndice() const { return _tileGridIndice; }
    void setTileGridIndice(int value);

    virtual void update(float dt) override;

protected:
    Image();
    virtual ~Image();

protected:
    virtual void rebuild();

    void tileFill(const VRectanglef& destRect, const VRectanglef& uvRect, float sourceW, float sourceH);
    void generateGrids(const VRectanglef& gridRect, const VRectanglef& uvRect);

    VColorRef _color;
    VRectanglef* _scale9Grid;
    bool _scaleByTile;
    int _tileGridIndice;
    FlipType _flip;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(Image);
};

NS_FGUI_END

#endif