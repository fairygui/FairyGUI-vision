#ifndef __GGRAPH_H__
#define __GGRAPH_H__

#include "FGUIMacros.h"
#include "GObject.h"
#include "core/Shape.h"
#include "gears/GearColor.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP GGraph : public GObject, public IColorGear
{
public:
    CREATE_FUNC(GGraph);

    void drawRect(float aWidth, float aHeight, int lineSize, const VColorRef& lineColor, const VColorRef& fillColor);
    void drawEllipse(float aWidth, float aHeight, const VColorRef& fillColor);
    bool isEmpty() const { return _shape->isEmpty(); }

    const VColorRef& getColor() const override;
    void setColor(const VColorRef& value) override;

protected:
    GGraph();
    virtual ~GGraph();

protected:
    virtual void handleInit() override;
    virtual void setup_BeforeAdd(TXMLElement* xml) override;

private:
    Shape *_shape;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GGraph);
};

NS_FGUI_END

#endif
