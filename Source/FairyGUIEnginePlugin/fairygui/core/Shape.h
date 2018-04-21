#ifndef __SHAPE_H__
#define __SHAPE_H__

#include "FGUIMacros.h"
#include "DisplayObject.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP Shape : public DisplayObject
{
public:
    CREATE_FUNC(Shape);

    void drawRect(int lineSize, const VColorRef&  lineColor, const VColorRef&  fillColor);
    void drawEllipse(const VColorRef& color);

    const VColorRef& getColor() const { return _fillColor; }
    void setColor(const VColorRef& value);

    bool isEmpty() const { return _type == 0; }
    void clear();

    virtual void update(float dt) override;
    virtual DisplayObject* hitTest(HitTestContext* context) override;

protected:
    Shape();
    virtual ~Shape();

private:
    int _type;
    int _lineSize;
    VColorRef _lineColor;
    VColorRef _fillColor;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(Shape);
};

NS_FGUI_END

#endif
