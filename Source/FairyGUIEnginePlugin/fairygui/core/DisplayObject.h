#ifndef __DISPLAYOBJECT_H__
#define __DISPLAYOBJECT_H__

#include "FGUIMacros.h"
#include "Node.h"
#include "NGraphics.h"

NS_FGUI_BEGIN

class RenderContext;
class HitTestContext;
class IHitTest;

class FGUI_IMPEXP DisplayObject : public Node
{
public:
    CREATE_FUNC(DisplayObject);

    float getX() const { return _position.x; };
    void setX(float value);
    float getY() const { return _position.y; };
    void setY(float value);
    const hkvVec2& getPosition() const { return _position; }
    void setPosition(float xv, float yv);
    hkvVec2 getLocation() const;
    void setLocation(float xv, float yv);

    float getWidth();
    void setWidth(float value);
    float getHeight();
    void setHeight(float value);
    const hkvVec2& getSize();
    void setSize(float wv, float hv);
    virtual void ensureSizeCorrect();

    float getScaleX() const { return _scale.x; }
    void setScaleX(float value);
    float getScaleY() const { return _scale.y; }
    void setScaleY(float value);
    const hkvVec2& getScale() const { return _scale; }
    void setScale(float xv, float yv);

    float getSkewX() const { return _skew.x; }
    void setSkewX(float value) { setSkew(value, _skew.y); }
    float getSkewY() const { return _skew.y; }
    void setSkewY(float value) { setSkew(_skew.x, value); }
    void setSkew(float xv, float yv);

    const hkvVec2& getPivot() const { return _pivot; }
    void setPivot(float xv, float yv);

    float getAlpha() const { return _alpha; }
    void setAlpha(float value);

    bool isVisible() const { return _visible; }
    void setVisible(bool value);

    float getRotation() const { return _rotation.z; }
    void setRotation(float value);

    bool isTouchable() const { return _touchable; }
    void setTouchable(bool value);

    bool isGrayed() const { return _grayed; }
    void setGrayed(bool value);

    const hkvMat4& getLocalToWorldMatrix();

    VRectanglef getBounds(DisplayObject* targetSpace);
    hkvVec2 localToGlobal(const hkvVec2& point);
    hkvVec2 globalToLocal(const hkvVec2& point);
    hkvVec2 transformPoint(const hkvVec2& point, DisplayObject* targetSpace);
    VRectanglef transformRect(const VRectanglef& rect, DisplayObject* targetSpace);

    NGraphics* getGraphics() const { return _graphics; }

    DisplayObject* getParent() const { return _parent; }
    virtual Node* getParentNode() const override { return _parent;  }
    void removeFromParent();

    DisplayObject* addChild(DisplayObject* child);
    DisplayObject* addChildAt(DisplayObject* child, int index);

    void removeChild(DisplayObject * child);
    void removeChildAt(int index);
    void removeChildren() { removeChildren(0, -1); }
    void removeChildren(int beginIndex, int endIndex);

    DisplayObject * getChildAt(int index) const;
    DisplayObject * getChild(const std::string& name) const;
    const Vector<DisplayObject*>& getChildren() const { return _children; }

    int getChildIndex(const DisplayObject* child) const;
    void setChildIndex(DisplayObject* child, int index);
    void swapChildren(DisplayObject* child1, DisplayObject* child2);
    void swapChildrenAt(int index1, int index2);

    int numChildren() const;
    bool isAncestorOf(const DisplayObject* obj) const;

    bool isTouchChildren() const { return _touchChildren; }
    void setTouchChildren(bool value) { _touchChildren = value; }

    bool isOpaque() const { return _opaque; }
    void setOpaque(bool value) { _opaque = value; }

    const VRectanglef& getClipRect() const;
    void setClipRect(const VRectanglef& value);

    IHitTest* getHitArea() const { return _hitArea; }
    void setHitArea(IHitTest* value) { _hitArea = value; }
    DisplayObject* hitTest(const hkvVec2& stagePoint, bool forTouch);

    virtual bool onStage() const override;

    virtual void update(float dt);
    virtual void onRender(RenderContext* context);

    std::string name;

protected:
    DisplayObject();
    virtual ~DisplayObject();

protected:
    virtual bool init();
    virtual void onSizeChanged(bool widthChanged, bool heightChanged);
    virtual DisplayObject* hitTest(HitTestContext* context);

    DisplayObject* internalHitTest(HitTestContext* context);
    DisplayObject* internalHitTestMask(HitTestContext* context);

    DisplayObject* _parent;
    NGraphics* _graphics;
    hkvVec2 _position;
    VRectanglef _contentRect;
    hkvVec2 _scale;
    hkvVec3 _rotation;
    hkvVec2 _skew;
    bool _visible;
    bool _touchable;
    bool _touchDisabled;
    hkvVec2 _pivot;
    hkvVec2 _pivotOffset;
    float _alpha;
    bool _grayed;
    bool _requireUpdateMesh;
    bool _outlineChanged;
    bool _opaque;
    bool _touchChildren;

    hkvMat4 _localToWorldMatrix;
    VRectanglef _renderRect;
    hkvVec2 _renderScale;

    Vector<DisplayObject*> _children;

private:
    void updatePivotOffset();
    void applyPivot();
    void validateMatrix(bool checkParent);

    VRectanglef* _clipRect;
    IHitTest* _hitArea;
    hkUint32 _matrixVersion;
    hkUint32 _parentMatrixVersion;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(DisplayObject);
};

NS_FGUI_END

#endif