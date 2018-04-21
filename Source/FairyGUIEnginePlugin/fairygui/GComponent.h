#ifndef __GCOMPONENT_H__
#define __GCOMPONENT_H__

#include "FGUIMacros.h"
#include "GObject.h"
#include "GController.h"
#include "Transition.h"
#include "ScrollPane.h"
#include "Margin.h"

NS_FGUI_BEGIN

class GGroup;

class FGUI_IMPEXP GComponent : public GObject
{
public:
    CREATE_FUNC(GComponent);

    GObject* addChild(GObject* child);
    virtual GObject* addChildAt(GObject* child, int index);

    void removeChild(GObject * child);
    virtual void removeChildAt(int index);
    void removeChildren() { removeChildren(0, -1); }
    void removeChildren(int beginIndex, int endIndex);

    GObject * getChildAt(int index) const;
    GObject * getChild(const std::string& name) const;
    GObject * getChildInGroup(const GGroup * group, const std::string& name) const;
    GObject * getChildById(const std::string& id) const;
    const Vector<GObject*>& getChildren() const { return _children; }

    int getChildIndex(const GObject* child) const;
    void setChildIndex(GObject* child, int index);
    int setChildIndexBefore(GObject* child, int index);
    void swapChildren(GObject* child1, GObject* child2);
    void swapChildrenAt(int index1, int index2);

    int numChildren() const;
    bool isAncestorOf(const GObject* obj) const;

    virtual bool isChildInView(GObject* child);
    virtual int getFirstChildInView();

    void addController(GController* c);
    GController* getControllerAt(int index) const;
    GController* getController(const std::string& name) const;
    const Vector<GController*>& getControllers() const { return _controllers; }
    void removeController(GController* c);
    void applyController(GController* c);
    void applyAllControllers();

    Transition* getTransition(const std::string& name) const;
    Transition* getTransitionAt(int index) const;
    const Vector<Transition*>& getTransitions() const { return _transitions; }

    bool getOpaque() const { return _displayObject->isOpaque(); }
    void setOpaque(bool value);

    const Margin& getMargin() { return _margin; }
    void setMargin(const Margin& value);

    ChildrenRenderOrder getChildrenRenderOrder() const { return _childrenRenderOrder; }
    void setChildrenRenderOrder(ChildrenRenderOrder value);
    int getApexIndex() const { return _apexIndex; }
    void setApexIndex(int value);

    DisplayObject* getMask() const;
    void setMask(DisplayObject* value, bool inverted = false);

    IHitTest* getHitArea() const { return _displayObject->getHitArea(); }
    void setHitArea(IHitTest* value) { _displayObject->setHitArea(value); }

    ScrollPane* getScrollPane() const { return _scrollPane; }

    float getViewWidth() const;
    void setViewWidth(float value);
    float getViewHeight() const;
    void setViewHeight(float value);

    void setBoundsChangedFlag();
    void ensureBoundsCorrect();

    virtual hkvVec2 getSnappingPosition(const hkvVec2& pt);

    //internal use
    void childSortingOrderChanged(GObject* child, int oldValue, int newValue);
    void childStateChanged(GObject * child);
    void adjustRadioGroupDepth(GObject* obj, GController* c);

    virtual void constructFromResource() override;
    void constructFromResource(std::vector<GObject*>* objectPool, int poolIndex);

    bool _buildingDisplayList;

protected:
    GComponent();
    virtual ~GComponent();

protected:
    virtual void constructFromXML(TXMLElement* xml);
    virtual void setup_AfterAdd(TXMLElement* xml) override;
    virtual void handleInit() override;
    virtual void handleSizeChanged() override;
    virtual void handleGrayedChanged() override;
    virtual void handleControllerChanged(GController* c) override;

    virtual void updateBounds();
    void setBounds(float ax, float ay, float aw, float ah);

    void setupOverflow(OverflowType overflow);
    void setupScroll(const Margin& scrollBarMargin,
        ScrollType scroll, ScrollBarDisplayType scrollBarDisplay, int flags,
        const std::string& vtScrollBarRes, const std::string& hzScrollBarRes,
        const std::string& headerRes, const std::string& footerRes);

    Vector<GObject*> _children;
    Vector<GController*> _controllers;
    Vector<Transition*> _transitions;
    DisplayObject* _container;
    ScrollPane* _scrollPane;
    Margin _margin;
    hkvVec2 _alignOffset;
    ChildrenRenderOrder _childrenRenderOrder;
    int _apexIndex;
    bool _boundsChanged;
    bool _trackBounds;

private:
    int getInsertPosForSortingChild(GObject * target);
    int moveChild(GObject* child, int oldIndex, int index);

    void doUpdateBounds(float);
    void buildNativeDisplayList(float);
    void scrollPaneUpdate(float dt);
    void refreshScrollPane(float);
    void onShowScrollBar(float);

    void onAddedToStage(EventContext* context);
    void onRemoveFromStage(EventContext* context);

    int _sortingChildCount;
    GController* _applyingController;

    friend class ScrollPane;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GComponent);
};

NS_FGUI_END

#endif
