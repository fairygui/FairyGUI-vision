#ifndef __SELECTION_SHAPE_H__
#define __SELECTION_SHAPE_H__

#include "FGUIMacros.h"
#include "DisplayObject.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP SelectionShape : public DisplayObject
{
public:
    CREATE_FUNC(SelectionShape);

    void setRects(const std::vector<VRectanglef>& rects);
    void setColor(const VColorRef& color);
    void clear();

    virtual void update(float dt) override;
    virtual DisplayObject* hitTest(HitTestContext* context) override;
   
protected:
    SelectionShape();
    virtual ~SelectionShape();

private:
    std::vector<VRectanglef> _rects;
    VColorRef _color;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(SelectionShape);
};

NS_FGUI_END

#endif
