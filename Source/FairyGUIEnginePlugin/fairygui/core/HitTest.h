#ifndef __HITTEST_H__
#define __HITTEST_H__

#include "FGUIMacros.h"
#include "utils/ByteArray.h"

NS_FGUI_BEGIN

class DisplayObject;

class FGUI_IMPEXP HitTestContext
{
public:
    HitTestContext();

    hkvVec2 screenPoint;
    bool forTouch;
};

class FGUI_IMPEXP IHitTest
{
public:
    virtual ~IHitTest() {}

    virtual bool hitTest(HitTestContext* context, DisplayObject* obj, hkvVec2& localPoint)
    {
        return true;
    }
};

class FGUI_IMPEXP RectHitTest : public IHitTest
{
public:
    RectHitTest();

    VRectanglef rect;
    virtual bool hitTest(HitTestContext* context, DisplayObject* obj, hkvVec2& localPoint) override;
};

class FGUI_IMPEXP PixelHitTestData
{
public:
    int pixelWidth;
    float scale;
    unsigned char* pixels;
    size_t pixelsLength;

    PixelHitTestData();
    ~PixelHitTestData();

    void load(ByteArray& ba);
};

class FGUI_IMPEXP PixelHitTest : public IHitTest
{
public:
    PixelHitTest(PixelHitTestData* data, int offsetX, int offsetY);

    virtual bool hitTest(HitTestContext* context, DisplayObject* obj, hkvVec2& localPoint) override;

    int offsetX;
    int offsetY;
    float scaleX;
    float scaleY;

private:
    PixelHitTestData* _data;
};

NS_FGUI_END

#endif