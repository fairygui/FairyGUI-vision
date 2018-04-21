#include "HitTest.h"
#include "DisplayObject.h"

USING_NS_FGUI;

HitTestContext::HitTestContext():screenPoint(0,0),forTouch(true)
{

}

RectHitTest::RectHitTest() :rect(0, 0, 0, 0)
{
}

bool RectHitTest::hitTest(HitTestContext* context, DisplayObject * obj, hkvVec2& localPoint)
{
    return rect.IsInside(localPoint);
}

PixelHitTestData::PixelHitTestData() :
    pixels(nullptr),
    pixelsLength(0),
    pixelWidth(0),
    scale(1)
{
}

PixelHitTestData::~PixelHitTestData()
{
    CC_SAFE_DELETE(pixels);
}

void PixelHitTestData::load(ByteArray & ba)
{
    ba.readInt();
    pixelWidth = ba.readInt();
    scale = 1.0f / ba.readByte();
    pixelsLength = ba.readInt();
    pixels = new unsigned char[pixelsLength];
    for (size_t i = 0; i < pixelsLength; i++)
        pixels[i] = ba.readByte();
}

PixelHitTest::PixelHitTest(PixelHitTestData * data, int offsetX, int offsetY) :
    offsetX(offsetX),
    offsetY(offsetY),
    scaleX(1),
    scaleY(1),
    _data(data)
{
}

bool PixelHitTest::hitTest(HitTestContext* context, DisplayObject* obj, hkvVec2& localPoint)
{
    int x = (int)floor((localPoint.x / scaleX - offsetX) * _data->scale);
    int y = (int)floor((localPoint.y / scaleY - offsetY) * _data->scale);
    if (x < 0 || y < 0 || x >= _data->pixelWidth)
        return false;

    ssize_t pos = y * _data->pixelWidth + x;
    ssize_t pos2 = pos / 8;
    ssize_t pos3 = pos % 8;

    if (pos2 >= 0 && pos2 < (ssize_t)_data->pixelsLength)
        return ((_data->pixels[pos2] >> pos3) & 0x1) > 0;
    else
        return false;
}