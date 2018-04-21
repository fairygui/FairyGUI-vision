#include "Shape.h"
#include "HitTest.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

Shape::Shape() :
    _type(0),
    _lineSize(1),
    _lineColor(V_RGBA_BLACK),
    _fillColor(V_RGBA_WHITE)
{
    _graphics = new NGraphics();
    _graphics->setWhiteTexture();
    _touchDisabled = true;
    _opaque = true;
}

Shape::~Shape()
{
}

void Shape::setColor(const VColorRef & color)
{
    if (color != _fillColor)
    {
        _fillColor = color;
        _requireUpdateMesh = true;
    }
}

void Shape::drawRect(int lineSize, const VColorRef & lineColor, const VColorRef & fillColor)
{
    _type = 1;
    _lineSize = lineSize;
    _lineColor = lineColor;
    _fillColor = fillColor;

    _touchDisabled = false;
    _requireUpdateMesh = true;
}

void Shape::drawEllipse(const VColorRef& color)
{
    _type = 2;
    _fillColor = color;

    _touchDisabled = false;
    _requireUpdateMesh = true;
}

void Shape::clear()
{
    _type = 0;
    _touchDisabled = true;
    _graphics->clearMesh();
}

void Shape::update(float dt)
{
    if (_requireUpdateMesh)
    {
        _requireUpdateMesh = false;
        if (_type != 0)
        {
            _graphics->clearMesh();
            if (_contentRect.GetSizeX() > 0 && _contentRect.GetSizeY() > 0)
            {
                if (_type == 1)
                    _graphics->drawRect(_contentRect, _lineSize, _lineColor, _fillColor);
                else if (_type == 2)
                    _graphics->drawEllipse(_contentRect, _fillColor);
                //else
                   // _graphics->drawPolygon(_polygonPoints, _fillColor, _colors);
            }
        }
    }

    DisplayObject::update(dt);
}

DisplayObject * Shape::hitTest(HitTestContext* context)
{
    if (_type == 2)
    {
        hkvVec2 localPoint = globalToLocal(context->screenPoint);
        if (!_contentRect.IsInside(localPoint))
            return nullptr;

        //圆形加多一个在圆内的判断
        float xx = localPoint.x - _contentRect.GetSizeX() * 0.5f;
        float yy = localPoint.y - _contentRect.GetSizeY() * 0.5f;
        if (hkvMath::pow2(xx / (_contentRect.GetSizeX() * 0.5f)) + hkvMath::pow2(yy / (_contentRect.GetSizeY() * 0.5f)) < 1)
            return this;
        else
            return nullptr;
    }
    else
        return DisplayObject::hitTest(context);
}

NS_FGUI_END
