#ifndef __SCROLLPANE_H__
#define __SCROLLPANE_H__

#include "FGUIMacros.h"
#include "Margin.h"

NS_FGUI_BEGIN

class GObject;
class GComponent;
class GScrollBar;
class GController;
class EventContext;
class DisplayObject;

class FGUI_IMPEXP ScrollPane : public Ref
{
public:
    ScrollPane(GComponent* owner,
        ScrollType scrollType,
        const Margin& scrollBarMargin,
        ScrollBarDisplayType scrollBarDisplay,
        int flags,
        const std::string& vtScrollBarRes,
        const std::string& hzScrollBarRes,
        const std::string& headerRes,
        const std::string& footerRes);
    virtual ~ScrollPane();

    GComponent* getOwner() const { return _owner; }
    GComponent* getHeader() const { return _header; }
    GComponent* getFooter() const { return _footer; }
    GScrollBar* getVtScrollBar() const { return _vtScrollBar; }
    GScrollBar* getHzScrollBar() const { return _hzScrollBar; }

    bool isBouncebackEffect() const { return _bouncebackEffect; }
    void setBouncebackEffect(bool value) { _bouncebackEffect = value; }

    bool isTouchEffect() const { return _touchEffect; }
    void setTouchEffect(bool value) { _touchEffect = value; }

    bool isInertiaDisabled() const { return _inertiaDisabled; }
    void setInertiaDisabled(bool value) { _inertiaDisabled = value; }

    float getScrollStep() const { return _scrollStep; }
    void setScrollStep(float value);

    bool isSnapToItem() const { return _snapToItem; }
    void setSnapToItem(bool value) { _snapToItem = value; }

    bool isPageMode() const { return _pageMode; }
    void setPageMode(bool value) { _pageMode = value; }

    GController* getPageController() const { return _pageController; }
    void setPageController(GController* value) { _pageController = value; }

    bool isMouseWheelEnabled() const { return _mouseWheelEnabled; }
    void setMouseWheelEnabled(bool value) { _mouseWheelEnabled = value; }

    float getDecelerationRate() const { return _decelerationRate; }
    void setDecelerationRate(float value) { _decelerationRate = value; }

    float getPosX() const { return _xPos; }
    void setPosX(float value, bool ani = false);
    float getPosY() const { return _yPos; }
    void setPosY(float value, bool ani = false);

    float getPercX() const;
    void setPercX(float value, bool ani = false);
    float getPercY() const;
    void setPercY(float value, bool ani = false);

    bool isBottomMost() const;
    bool isRightMost() const;

    void scrollLeft(float ratio = 1, bool ani = false);
    void scrollRight(float ratio = 1, bool ani = false);
    void scrollUp(float ratio = 1, bool ani = false);
    void scrollDown(float ratio = 1, bool ani = false);
    void scrollTop(bool ani = false);
    void scrollBottom(bool ani = false);
    void scrollToView(GObject* obj, bool ani = false, bool setFirst = false);
    void scrollToView(const VRectanglef& rect, bool ani = false, bool setFirst = false);
    bool isChildInView(GObject* obj) const;

    int getPageX() const;
    void setPageX(int value, bool ani = false);
    int getPageY() const;
    void setPageY(int value, bool ani = false);

    float getScrollingPosX() const;
    float getScrollingPosY() const;

    const hkvVec2& getContentSize() const { return _contentSize; }
    const hkvVec2& getViewSize() const { return _viewSize; }

    void lockHeader(int size);
    void lockFooter(int size);

    void cancelDragging();
    static ScrollPane* getDraggingPane() { return _draggingPane; }

private:

    void onOwnerSizeChanged();
    void adjustMaskContainer();
    void setContentSize(float wv, float hv);
    void changeContentSizeOnScrolling(float deltaWidth, float deltaHeight, float deltaPosX, float deltaPosY);
    void setViewWidth(float value);
    void setViewHeight(float value);
    void setSize(float wv, float hv);
    void handleSizeChanged();

    void handleControllerChanged(GController* c);
    void updatePageController();

    void posChanged(bool ani);
    void refresh();
    void refresh2();

    void syncScrollBar(bool end = false);
    void showScrollBar(bool show);
    void onShowScrollBar();

    float getLoopPartSize(float division, int axis);
    bool loopCheckingCurrent();
    void loopCheckingTarget(hkvVec2& endPos);
    void loopCheckingTarget(hkvVec2& endPos, int axis);
    void loopCheckingNewPos(float& value, int axis);
    void alignPosition(hkvVec2& pos, bool inertialScrolling);
    float alignByPage(float pos, int axis, bool inertialScrolling);
    hkvVec2 updateTargetAndDuration(const hkvVec2& orignPos);
    float updateTargetAndDuration(float pos, int axis);
    void fixDuration(int axis, float oldChange);
    void killTween();
    void tweenUpdate(float dt);
    float runTween(int axis, float dt);

    void checkRefreshBar();

    void onTouchBegin(EventContext* context);
    void onTouchMove(EventContext* context);
    void onTouchEnd(EventContext* context);
    void onMouseWheel(EventContext* context);
    void onRollOver(EventContext* context);
    void onRollOut(EventContext* context);

    ScrollType _scrollType;
    float _scrollStep;
    float _mouseWheelStep;
    Margin _scrollBarMargin;
    bool _bouncebackEffect;
    bool _touchEffect;
    bool _scrollBarDisplayAuto;
    bool _vScrollNone;
    bool _hScrollNone;
    bool _needRefresh;
    int _refreshBarAxis;
    bool _displayOnLeft;
    bool _snapToItem;
    bool _displayInDemand;
    bool _mouseWheelEnabled;
    bool _inertiaDisabled;
    bool _maskDisabled;
    float _decelerationRate;
    bool _pageMode;

    float _xPos;
    float _yPos;

    hkvVec2 _viewSize;
    hkvVec2 _contentSize;
    hkvVec2 _overlapSize;
    hkvVec2 _pageSize;

    hkvVec2 _containerPos;
    hkvVec2 _beginTouchPos;
    hkvVec2 _lastTouchPos;
    hkvVec2 _lastTouchGlobalPos;
    hkvVec2 _velocity;
    float _velocityScale;
    clock_t _lastMoveTime;
    bool _isMouseMoved;
    bool _isHoldAreaDone;
    int _aniFlag;
    bool _scrollBarVisible;
    int _loop;

    int _headerLockedSize;
    int _footerLockedSize;

    int _tweening;
    hkvVec2 _tweenStart;
    hkvVec2 _tweenChange;
    hkvVec2 _tweenTime;
    hkvVec2 _tweenDuration;

    GComponent* _owner;
    DisplayObject* _maskContainer;
    DisplayObject* _container;
    GScrollBar* _hzScrollBar;
    GScrollBar* _vtScrollBar;
    GComponent* _header;
    GComponent* _footer;
    GController* _pageController;

    static int _gestureFlag;
    static ScrollPane* _draggingPane;

    friend class GComponent;
    friend class GList;
};

NS_FGUI_END

#endif
