#include "GGraph.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

GGraph::GGraph() :
    _shape(nullptr)
{
}

GGraph::~GGraph()
{
}

void GGraph::handleInit()
{
    _shape = Shape::create();
    _shape->retain();

    _displayObject = _shape;
}

void GGraph::drawRect(float aWidth, float aHeight, int lineSize, const VColorRef& lineColor, const VColorRef& fillColor)
{
    _shape->drawRect(lineSize, lineColor, fillColor);
    setSize(aWidth, aHeight);
}

void GGraph::drawEllipse(float aWidth, float aHeight, const VColorRef& fillColor)
{
    _shape->drawEllipse(fillColor);
    setSize(aWidth, aHeight);
}

const VColorRef& GGraph::getColor() const
{
    return _shape->getColor();
}

void GGraph::setColor(const VColorRef& value)
{
    _shape->setColor(value);
}

void GGraph::setup_BeforeAdd(TXMLElement * xml)
{
    GObject::setup_BeforeAdd(xml);

    int type = 0;
    const char* p = xml->Attribute("type");
    if (p)
    {
        if (strcmp(p, "rect") == 0)
            type = 1;
        else if (strcmp(p, "eclipse") == 0)
            type = 2;
    }

    if (type != 0)
    {
        int lineSize = 1;
        VColorRef lineColor = V_RGBA_BLACK;
        VColorRef fillColor = V_RGBA_WHITE;
        p = xml->Attribute("lineSize");
        if (p)
            lineSize = atoi(p);

        p = xml->Attribute("lineColor");
        if (p)
            lineColor = ToolSet::convertFromHtmlColor(p);

        p = xml->Attribute("fillColor");
        if (p)
            fillColor = ToolSet::convertFromHtmlColor(p);

        if (type == 1)
            drawRect(_size.x, _size.y, lineSize, lineColor, fillColor);
        else
            drawEllipse(_size.x, _size.y, fillColor);
    }
}

NS_FGUI_END