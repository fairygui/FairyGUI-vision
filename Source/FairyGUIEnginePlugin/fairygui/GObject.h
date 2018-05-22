#ifndef __GOBJECT_H__
#define __GOBJECT_H__

#include "FGUIMacros.h"
#include "core/DisplayObject.h"
#include "Relations.h"

NS_FGUI_BEGIN

class GComponent;
class GGroup;
class GRoot;
class GController;
class GearBase;
class PackageItem;

class FGUI_IMPEXP GObject : public Node
{
public:
    static GObject* getDraggingObject() { return _draggingObject; }

    CREATE_FUNC(GObject);

    float getX() const { return _position.x; };
    void setX(float value);
    float getY() const { return _position.y; };
    void setY(float value);
    const hkvVec2& getPosition()const { return _position; }
    void setPosition(float xv, float yv);
    float getXMin() const;
    void setXMin(float value);
    float getYMin() const;
    void setYMin(float value);

    bool isPixelSnapping() const { return _pixelSnapping; }
    void setPixelSnapping(bool value);

    float getWidth() const { return _size.x; }
    void setWidth(float value) { setSize(value, _rawSize.y); }
    float getHeight() const { return _size.y; }
    void setHeight(float value) { setSize(_rawSize.x, value); }
    const hkvVec2& getSize() const { return _size; }
    void setSize(float wv, float hv, bool ignorePivot = false);

    void center(bool restraint = false);
    void makeFullScreen();

    const hkvVec2& getPivot() const { return _pivot; }
    void setPivot(float xv, float yv, bool asAnchor = false);
    bool isPivotAsAnchor() const { return _pivotAsAnchor; }

    float getScaleX() const { return _scale.x; }
    void setScaleX(float value) { setScale(value, _scale.y); }
    float getScaleY() const { return _scale.y; }
    void setScaleY(float value) { setScale(_scale.x, value); }
    const hkvVec2& getScale() const { return _scale; }
    void setScale(float xv, float yv);

    float getSkewX() const { return _displayObject->getSkewX(); }
    void setSkewX(float value);

    float getSkewY() const { return _displayObject->getSkewY(); }
    void setSkewY(float value);

    float getRotation() const { return _rotation; }
    void setRotation(float value);

    float getAlpha() const { return _alpha; }
    void setAlpha(float value);

    bool isGrayed() const { return _grayed; }
    void setGrayed(bool value);

    bool isVisible() const { return _visible; }
    void setVisible(bool value);

    bool isTouchable() const { return _touchable; }
    void setTouchable(bool value);

    int getSortingOrder() const { return _sortingOrder; }
    void setSortingOrder(int value);

    GGroup* getGroup() const { return _group; }
    void setGroup(GGroup* value);

    virtual const std::string& getText() const;
    virtual void setText(const std::string& text);

    virtual const std::string& getIcon() const;
    virtual void setIcon(const std::string& text);

    const std::string& getTooltips() const { return _tooltips; }
    void setTooltips(const std::string& value);

    void* getData() const { return _data; };
    void setData(void* value) { _data = value; }
    const Value& getCustomData() const { return _customData; }
    void setCustomData(const Value& value) { _customData = value; }

    bool isDraggable() const { return _draggable; }
    void setDraggable(bool value);
    VRectanglef* getDragBounds() const { return _dragBounds; }
    void setDragBounds(const VRectanglef& value);

    void startDrag(int touchId = -1);
    void stopDrag();

    std::string getResourceURL() const;

    PackageItem* getPackageItem()const { return _packageItem; }

    hkvVec2 globalToLocal(const hkvVec2& pt);
    VRectanglef globalToLocal(const VRectanglef& rect);
    hkvVec2 localToGlobal(const hkvVec2& pt);
    VRectanglef localToGlobal(const VRectanglef& rect);
    VRectanglef transformRect(const VRectanglef& rect, GObject* targetSpace);

    Relations* relations() { return _relations; }
    void addRelation(GObject* target, RelationType relationType, bool usePercent = false);
    void removeRelation(GObject* target, RelationType relationType);

    GearBase* getGear(int index);
    bool checkGearController(int index, GController* c);
    uint32_t addDisplayLock();
    void releaseDisplayLock(uint32_t token);

    GComponent* getParent() const { return _parent; }
    virtual Node* getParentNode() const override { return (Node*)_parent; }
    DisplayObject* displayObject() const { return _displayObject; }
    GRoot* getRoot() const;
    bool onStage() const;
    void removeFromParent();

    void addClickListener(const EventCallback& callback) { addListener(UIEventType::Click, callback); }
    void addClickListener(const EventCallback& callback, const EventTag& tag) { addListener(UIEventType::Click, callback, tag); }
    void removeClickListener(const EventTag& tag) { removeListener(UIEventType::Click, tag); }

    virtual void constructFromResource();

    template<typename T> T* as();

    std::string id;
    std::string name;
    hkvVec2 sourceSize;
    hkvVec2 initSize;
    hkvVec2 minSize;
    hkvVec2 maxSize;

    //internal use
    bool _underConstruct;
    bool _gearLocked;

protected:
    GObject();
    virtual ~GObject();

protected:
    GComponent* _parent;
    DisplayObject* _displayObject;
    PackageItem* _packageItem;
    int _sizeImplType;

    virtual void handleInit();
    virtual void handleSizeChanged();
    virtual void handleScaleChanged();
    virtual void handleGrayedChanged();
    virtual void handlePositionChanged();
    virtual void handleControllerChanged(GController* c);
    virtual void handleAlphaChanged();
    virtual void handleVisibleChanged();

    virtual void setup_BeforeAdd(TXMLElement* xml);
    virtual void setup_AfterAdd(TXMLElement* xml);

    bool init();

    void updateGear(int index);
    void checkGearDisplay();

    void setSizeDirectly(float wv, float hv);

    hkvVec2 _position;
    hkvVec2 _size;
    hkvVec2 _rawSize;
    hkvVec2 _pivot;
    hkvVec2 _scale;
    bool _pivotAsAnchor;
    float _alpha;
    float _rotation;
    bool _visible;
    bool _touchable;
    bool _grayed;

private:
    bool internalVisible() const;
    bool internalVisible2() const;
    void updateGearFromRelations(int index, float dx, float dy);

    void initDrag();
    void dragBegin(int touchId);
    void dragEnd();
    void onTouchBegin(EventContext* context);
    void onTouchMove(EventContext* context);
    void onTouchEnd(EventContext* context);
    void onRollOver(EventContext* context);
    void onRollOut(EventContext* context);

    bool _internalVisible;
    bool _handlingController;
    bool _draggable;
    int _sortingOrder;
    bool _focusable;
    std::string _tooltips;
    bool _pixelSnapping;
    GGroup* _group;
    float _sizePercentInGroup;
    Relations* _relations;
    GearBase* _gears[8];
    void * _data;
    Value _customData;
    hkvVec2 _dragTouchStartPos;
    VRectanglef* _dragBounds;

    static GObject* _draggingObject;

    friend class GComponent;
    friend class GGroup;
    friend class RelationItem;
    friend class UIObjectFactory;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GObject);
};

template<typename T>
inline T* GObject::as()
{
    return dynamic_cast<T*>(this);
}


NS_FGUI_END

#endif
