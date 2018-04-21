#include "SelectionShape.h"
#include "HitTest.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

SelectionShape::SelectionShape()
{
    _graphics = new NGraphics();
    _graphics->setWhiteTexture();
    _color = V_RGBA_WHITE;
    _opaque = true;
}

SelectionShape::~SelectionShape()
{

}

void SelectionShape::setRects(const std::vector<VRectanglef>& rects)
{
    _rects.clear();
    for (auto &it : rects)
        _rects.push_back(it);
    int count = _rects.size();
    if (count > 0)
    {
        _contentRect = _rects[0];
        for (int i = 1; i < count; i++)
            _contentRect = ToolSet::unionRect(_contentRect, _rects[i]);
    }
    else
        _contentRect.Set(0, 0, 0, 0);
    _requireUpdateMesh = true;
}

void SelectionShape::setColor(const VColorRef & color)
{
    if (color != _color)
    {
        _color = color;
        _graphics->tint(color);
    }
}

void SelectionShape::clear()
{
    if (!_rects.empty())
    {
        _rects.clear();
        _contentRect.Set(0, 0, 0, 0);
        onSizeChanged(true, true);
        _requireUpdateMesh = true;
    }
}

void SelectionShape::update(float dt)
{
    if (_requireUpdateMesh)
    {
        _requireUpdateMesh = false;

        _graphics->clearMesh();
        for (auto &it : _rects)
            _graphics->addQuad(it, NGraphics::FULL_UV, _color);
    }

    DisplayObject::update(dt);
}

DisplayObject * SelectionShape::hitTest(HitTestContext* context)
{
    int count = _rects.size();
    if (count == 0)
        return nullptr;

    hkvVec2 localPoint = globalToLocal(context->screenPoint);
    if (!_contentRect.IsInside(localPoint))
        return nullptr;

    for (int i = 0; i < count; i++)
    {
        if (_rects[i].IsInside(localPoint))
            return this;
    }

    return nullptr;
}

NS_FGUI_END