#include "ScrollPane.h"
#include "UIPackage.h"
#include "GList.h"
#include "GScrollBar.h"
#include "UIConfig.h"
#include "FGUIManager.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

ScrollPane* ScrollPane::_draggingPane = nullptr;
int ScrollPane::_gestureFlag = 0;

static const float TWEEN_TIME_GO = 0.5f; //tween time for SetPos(ani)
static const float TWEEN_TIME_DEFAULT = 0.3f; //min tween time for inertial scroll
static const float PULL_RATIO = 0.5f; //pull down/up ratio

static inline float sp_getField(const hkvVec2& pt, int axis) { return axis == 0 ? pt.x : pt.y; }
static void sp_setField(hkvVec2& pt, int axis, float value) { if (axis == 0) pt.x = value; else pt.y = value; }
static void sp_incField(hkvVec2& pt, int axis, float value) { if (axis == 0) pt.x += value; else pt.y += value; }

static inline float sp_EaseFunc(float t, float d)
{
    t = t / d - 1;
    return t * t * t + 1;//cubicOut
}

ScrollPane::ScrollPane(GComponent* owner,
    ScrollType scrollType,
    const Margin& scrollBarMargin,
    ScrollBarDisplayType scrollBarDisplay,
    int flags,
    const std::string& vtScrollBarRes,
    const std::string& hzScrollBarRes,
    const std::string& headerRes,
    const std::string& footerRes) :
    _vtScrollBar(nullptr),
    _hzScrollBar(nullptr),
    _header(nullptr),
    _footer(nullptr),
    _pageController(nullptr),
    _needRefresh(false),
    _refreshBarAxis(0),
    _aniFlag(0),
    _loop(0),
    _headerLockedSize(0),
    _footerLockedSize(0),
    _vScrollNone(false),
    _hScrollNone(false),
    _tweening(0),
    _xPos(0),
    _yPos(0),
    _viewSize(0, 0),
    _contentSize(0, 0),
    _overlapSize(0, 0),
    _pageSize(0, 0),
    _containerPos(0, 0),
    _beginTouchPos(0, 0),
    _lastTouchPos(0, 0),
    _lastTouchGlobalPos(0, 0),
    _velocity(0, 0)
{
    _owner = owner;

    _maskContainer = DisplayObject::create();
    _owner->displayObject()->addChild(_maskContainer);

    _container = _owner->displayObject()->getChildAt(0);
    _container->setPosition(0, 0);
    _maskContainer->addChild(_container);

    _scrollBarMargin = scrollBarMargin;
    _scrollType = scrollType;
    _scrollStep = UIConfig::defaultScrollStep;
    _mouseWheelStep = _scrollStep * 2;
    _decelerationRate = UIConfig::defaultScrollDecelerationRate;

    _displayOnLeft = (flags & 1) != 0;
    _snapToItem = (flags & 2) != 0;
    _displayInDemand = (flags & 4) != 0;
    _pageMode = (flags & 8) != 0;
    if ((flags & 16) != 0)
        _touchEffect = true;
    else if ((flags & 32) != 0)
        _touchEffect = false;
    else
        _touchEffect = UIConfig::defaultScrollTouchEffect;
    if ((flags & 64) != 0)
        _bouncebackEffect = true;
    else if ((flags & 128) != 0)
        _bouncebackEffect = false;
    else
        _bouncebackEffect = UIConfig::defaultScrollBounceEffect;
    _inertiaDisabled = (flags & 256) != 0;
    _maskDisabled = (flags & 512) != 0;

    _scrollBarVisible = true;
    _mouseWheelEnabled = true;
    _pageSize.set(0, 0);

    if (scrollBarDisplay == ScrollBarDisplayType::DEFAULT)
    {
#ifdef CC_PLATFORM_PC
        scrollBarDisplay = UIConfig::defaultScrollBarDisplay;
#else
        scrollBarDisplay = ScrollBarDisplayType::AUTO;
#endif
    }

    if (scrollBarDisplay != ScrollBarDisplayType::HIDDEN)
    {
        if (_scrollType == ScrollType::BOTH || _scrollType == ScrollType::VERTICAL)
        {
            const std::string& res = vtScrollBarRes.size() == 0 ? UIConfig::verticalScrollBar : vtScrollBarRes;
            if (res.length() > 0)
            {
                _vtScrollBar = dynamic_cast<GScrollBar*>(UIPackage::createObjectFromURL(res));
                if (_vtScrollBar == nullptr)
                    CCLOGWARN("FairyGUI: cannot create scrollbar from %s", res.c_str());
                else
                {
                    _vtScrollBar->retain();
                    _vtScrollBar->setScrollPane(this, true);
                    _owner->displayObject()->addChild(_vtScrollBar->displayObject());
                }
            }
        }
        if (_scrollType == ScrollType::BOTH || _scrollType == ScrollType::HORIZONTAL)
        {
            const std::string& res = hzScrollBarRes.length() == 0 ? UIConfig::horizontalScrollBar : hzScrollBarRes;
            if (res.length() > 0)
            {
                _hzScrollBar = dynamic_cast<GScrollBar*>(UIPackage::createObjectFromURL(res));
                if (_hzScrollBar == nullptr)
                    CCLOGWARN("FairyGUI: cannot create scrollbar from %s", res.c_str());
                else
                {
                    _hzScrollBar->retain();
                    _hzScrollBar->setScrollPane(this, false);
                    _owner->displayObject()->addChild(_hzScrollBar->displayObject());
                }
            }
        }

        _scrollBarDisplayAuto = scrollBarDisplay == ScrollBarDisplayType::AUTO;
        if (_scrollBarDisplayAuto)
        {
            if (_vtScrollBar != nullptr)
                _vtScrollBar->setVisible(false);
            if (_hzScrollBar != nullptr)
                _hzScrollBar->setVisible(false);
            _scrollBarVisible = false;

            _owner->addListener(UIEventType::RollOver, CALLBACK_1(ScrollPane::onRollOver, this));
            _owner->addListener(UIEventType::RollOut, CALLBACK_1(ScrollPane::onRollOut, this));
        }
    }
    else
        _mouseWheelEnabled = false;

    if (headerRes.length() > 0)
    {
        _header = dynamic_cast<GComponent*>(UIPackage::createObjectFromURL(headerRes));
        if (_header == nullptr)
            CCLOGWARN("FairyGUI: cannot create scrollPane header from %s", headerRes.c_str());
        else
            _header->retain();
    }

    if (footerRes.length() > 0)
    {
        _footer = dynamic_cast<GComponent*>(UIPackage::createObjectFromURL(footerRes));
        if (_footer == nullptr)
            CCLOGWARN("FairyGUI: cannot create scrollPane footer from %s", footerRes.c_str());
        else
            _footer->retain();
    }

    if (_header != nullptr || _footer != nullptr)
        _refreshBarAxis = (_scrollType == ScrollType::BOTH || _scrollType == ScrollType::VERTICAL) ? 1 : 0;

    setSize(owner->getWidth(), owner->getHeight());

    _owner->addListener(UIEventType::MouseWheel, CALLBACK_1(ScrollPane::onMouseWheel, this));
    _owner->addListener(UIEventType::TouchBegin, CALLBACK_1(ScrollPane::onTouchBegin, this));
    _owner->addListener(UIEventType::TouchMove, CALLBACK_1(ScrollPane::onTouchMove, this));
    _owner->addListener(UIEventType::TouchEnd, CALLBACK_1(ScrollPane::onTouchEnd, this));
}

ScrollPane::~ScrollPane()
{
    CC_SAFE_RELEASE(_hzScrollBar);
    CC_SAFE_RELEASE(_vtScrollBar);
    CC_SAFE_RELEASE(_header);
    CC_SAFE_RELEASE(_footer);
}

void ScrollPane::setScrollStep(float value)
{
    _scrollStep = value;
    if (_scrollStep == 0)
        _scrollStep = UIConfig::defaultScrollStep;
    _mouseWheelStep = _scrollStep * 2;
}

void ScrollPane::setPosX(float value, bool ani)
{
    _owner->ensureBoundsCorrect();

    if (_loop == 1)
        loopCheckingNewPos(value, 0);

    value = hkvMath::clamp(value, 0.0f, _overlapSize.x);
    if (value != _xPos)
    {
        _xPos = value;
        posChanged(ani);
    }
}

void ScrollPane::setPosY(float value, bool ani)
{
    _owner->ensureBoundsCorrect();

    if (_loop == 2)
        loopCheckingNewPos(value, 1);

    value = hkvMath::clamp(value, 0.0f, _overlapSize.y);
    if (value != _yPos)
    {
        _yPos = value;
        posChanged(ani);
    }
}

float ScrollPane::getPercX() const
{
    return _overlapSize.x == 0 ? 0 : _xPos / _overlapSize.x;
}

void ScrollPane::setPercX(float value, bool ani)
{
    _owner->ensureBoundsCorrect();
    setPosX(_overlapSize.x * hkvMath::clamp(value, 0.0f, 1.0f), ani);
}

float ScrollPane::getPercY() const
{
    return _overlapSize.y == 0 ? 0 : _yPos / _overlapSize.y;
}

void ScrollPane::setPercY(float value, bool ani)
{
    _owner->ensureBoundsCorrect();
    setPosY(_overlapSize.y * hkvMath::clamp(value, 0.0f, 1.0f), ani);
}

bool ScrollPane::isBottomMost() const
{
    return _yPos == _overlapSize.y || _overlapSize.y == 0;
}

bool ScrollPane::isRightMost() const
{
    return _xPos == _overlapSize.x || _overlapSize.x == 0;
}

void ScrollPane::scrollLeft(float ratio, bool ani)
{
    if (_pageMode)
        setPosX(_xPos - _pageSize.x * ratio, ani);
    else
        setPosX(_xPos - _scrollStep * ratio, ani);
}

void ScrollPane::scrollRight(float ratio, bool ani)
{
    if (_pageMode)
        setPosX(_xPos + _pageSize.x * ratio, ani);
    else
        setPosX(_xPos + _scrollStep * ratio, ani);
}

void ScrollPane::scrollUp(float ratio, bool ani)
{
    if (_pageMode)
        setPosY(_yPos - _pageSize.y * ratio, ani);
    else
        setPosY(_yPos - _scrollStep * ratio, ani);
}

void ScrollPane::scrollDown(float ratio, bool ani)
{
    if (_pageMode)
        setPosY(_yPos + _pageSize.y * ratio, ani);
    else
        setPosY(_yPos + _scrollStep * ratio, ani);
}

void ScrollPane::scrollTop(bool ani)
{
    setPercY(0, ani);
}

void ScrollPane::scrollBottom(bool ani)
{
    setPercY(1, ani);
}

void ScrollPane::scrollToView(GObject * obj, bool ani, bool setFirst)
{
    _owner->ensureBoundsCorrect();
    if (_needRefresh)
        refresh();

    VRectanglef rect(obj->getX(), obj->getY(), obj->getX() + obj->getWidth(), obj->getY() + obj->getHeight());
    if (obj->getParent() != _owner)
        rect = obj->getParent()->transformRect(rect, _owner);
    scrollToView(rect, ani, setFirst);
}

void ScrollPane::scrollToView(const VRectanglef & rect, bool ani, bool setFirst)
{
    _owner->ensureBoundsCorrect();
    if (_needRefresh)
        refresh();

    if (_overlapSize.y > 0)
    {
        float bottom = _yPos + _viewSize.y;
        if (setFirst || rect.m_vMin.y <= _yPos || rect.GetSizeY() >= _viewSize.y)
        {
            if (_pageMode)
                setPosY(floor(rect.m_vMin.y / _pageSize.y) * _pageSize.y, ani);
            else
                setPosY(rect.m_vMin.y, ani);
        }
        else if (rect.m_vMax.y > bottom)
        {
            if (_pageMode)
                setPosY(floor(rect.m_vMin.y / _pageSize.y) * _pageSize.y, ani);
            else if (rect.GetSizeY() <= _viewSize.y / 2)
                setPosY(rect.m_vMin.y + rect.GetSizeY() * 2 - _viewSize.y, ani);
            else
                setPosY(rect.m_vMax.y - _viewSize.y, ani);
        }
    }
    if (_overlapSize.x > 0)
    {
        float right = _xPos + _viewSize.x;
        if (setFirst || rect.m_vMin.x <= _xPos || rect.GetSizeX() >= _viewSize.x)
        {
            if (_pageMode)
                setPosX(floor(rect.m_vMin.x / _pageSize.x) * _pageSize.x, ani);
            setPosX(rect.m_vMin.x, ani);
        }
        else if (rect.m_vMax.x > right)
        {
            if (_pageMode)
                setPosX(floor(rect.m_vMin.x / _pageSize.x) * _pageSize.x, ani);
            else if (rect.GetSizeX() <= _viewSize.x / 2)
                setPosX(rect.m_vMin.x + rect.GetSizeX() * 2 - _viewSize.x, ani);
            else
                setPosX(rect.m_vMax.x - _viewSize.x, ani);
        }
    }

    if (!ani && _needRefresh)
        refresh();
}

bool ScrollPane::isChildInView(GObject * obj) const
{
    if (_overlapSize.y > 0)
    {
        float dist = obj->getY() + _container->getY();
        if (dist <= -obj->getHeight() || dist >= _viewSize.y)
            return false;
    }
    if (_overlapSize.x > 0)
    {
        float dist = obj->getX() + _container->getX();
        if (dist <= -obj->getWidth() || dist >= _viewSize.x)
            return false;
    }

    return true;
}

int ScrollPane::getPageX() const
{
    if (!_pageMode)
        return 0;

    int page = (int)floor(_xPos / _pageSize.x);
    if (_xPos - page * _pageSize.x > _pageSize.x * 0.5f)
        page++;

    return page;
}

void ScrollPane::setPageX(int value, bool ani)
{
    if (!_pageMode)
        return;

    if (_overlapSize.x > 0)
        setPosX(value * _pageSize.x, ani);
}

int ScrollPane::getPageY() const
{
    if (!_pageMode)
        return 0;

    int page = (int)floor(_yPos / _pageSize.y);
    if (_yPos - page * _pageSize.y > _pageSize.y * 0.5f)
        page++;

    return page;
}

void ScrollPane::setPageY(int value, bool ani)
{
    if (_overlapSize.y > 0)
        setPosY(value * _pageSize.y, ani);
}

float ScrollPane::getScrollingPosX() const
{
    return hkvMath::clamp(-_container->getX(), 0.0f, _overlapSize.x);
}

float ScrollPane::getScrollingPosY() const
{
    return hkvMath::clamp(-_container->getY(), 0.0f, _overlapSize.y);
}

void ScrollPane::setViewWidth(float value)
{
    value = value + _owner->_margin.left + _owner->_margin.right;
    if (_vtScrollBar != nullptr)
        value += _vtScrollBar->getWidth();
    _owner->setWidth(value);
}

void ScrollPane::setViewHeight(float value)
{
    value = value + _owner->_margin.top + _owner->_margin.bottom;
    if (_hzScrollBar != nullptr)
        value += _hzScrollBar->getHeight();
    _owner->setHeight(value);
}

void ScrollPane::lockHeader(int size)
{
    if (_headerLockedSize == size)
        return;

    hkvVec2 cpos = _container->getPosition();

    _headerLockedSize = size;
    if (!_owner->isDispatchingEvent(UIEventType::PullDownRelease)
        && sp_getField(cpos, _refreshBarAxis) >= 0)
    {
        _tweenStart = cpos;
        _tweenChange.setZero();
        sp_setField(_tweenChange, _refreshBarAxis, _headerLockedSize - sp_getField(_tweenStart, _refreshBarAxis));
        _tweenDuration.set(TWEEN_TIME_DEFAULT, TWEEN_TIME_DEFAULT);
        _tweenTime.setZero();
        _tweening = 2;

        _owner->schedule(SCHEDULE_SELECTOR(GComponent::scrollPaneUpdate), 0);
    }
}

void ScrollPane::lockFooter(int size)
{
    if (_footerLockedSize == size)
        return;

    hkvVec2 cpos = _container->getPosition();

    _footerLockedSize = size;
    if (!_owner->isDispatchingEvent(UIEventType::PullUpRelease)
        && sp_getField(cpos, _refreshBarAxis) >= 0)
    {
        _tweenStart = cpos;
        _tweenChange.setZero();
        float max = sp_getField(_overlapSize, _refreshBarAxis);
        if (max == 0)
            max = MAX(sp_getField(_contentSize, _refreshBarAxis) + _footerLockedSize - sp_getField(_viewSize, _refreshBarAxis), 0.0f);
        else
            max += _footerLockedSize;
        sp_setField(_tweenChange, _refreshBarAxis, -max - sp_getField(_tweenStart, _refreshBarAxis));
        _tweenDuration.set(TWEEN_TIME_DEFAULT, TWEEN_TIME_DEFAULT);
        _tweenTime.setZero();
        _tweening = 2;

        _owner->schedule(SCHEDULE_SELECTOR(GComponent::scrollPaneUpdate), 0);
    }
}

void ScrollPane::cancelDragging()
{
    if (_draggingPane == this)
        _draggingPane = nullptr;

    _gestureFlag = 0;
    _isMouseMoved = false;
}

void ScrollPane::handleControllerChanged(GController * c)
{
    if (_pageController == c)
    {
        if (_scrollType == ScrollType::HORIZONTAL)
            setPageX(c->getSelectedIndex());
        else
            setPageY(c->getSelectedIndex());
    }
}

void ScrollPane::updatePageController()
{
    
    if (_pageController != nullptr && !_pageController->isChanging())
    {
        int index;
        if (_scrollType == ScrollType::HORIZONTAL)
            index = getPageX();
        else
            index = getPageY();
        if (index < _pageController->getPageCount())
        {
            GController* c = _pageController;
            _pageController = nullptr; //avoid calling handleControllerChanged
            c->setSelectedIndex(index);
            _pageController = c;
        }
    }
}

void ScrollPane::adjustMaskContainer()
{
    float mx, my;
    if (_displayOnLeft && _vtScrollBar != nullptr)
        mx = floor(_owner->_margin.left + _vtScrollBar->getWidth());
    else
        mx = floor(_owner->_margin.left);
    my = floor(_owner->_margin.top);
    mx += _owner->_alignOffset.x;
    my += _owner->_alignOffset.y;

    _maskContainer->setPosition(mx, my);
}

void ScrollPane::onOwnerSizeChanged()
{
    setSize(_owner->getWidth(), _owner->getHeight());
    posChanged(false);
}

void ScrollPane::setSize(float wv, float hv)
{
    if (_hzScrollBar != nullptr)
    {
        _hzScrollBar->setY(hv - _hzScrollBar->getHeight());
        if (_vtScrollBar != nullptr)
        {
            _hzScrollBar->setWidth(wv - _vtScrollBar->getWidth() - _scrollBarMargin.left - _scrollBarMargin.right);
            if (_displayOnLeft)
                _hzScrollBar->setX(_scrollBarMargin.left + _vtScrollBar->getWidth());
            else
                _hzScrollBar->setX(_scrollBarMargin.left);
        }
        else
        {
            _hzScrollBar->setWidth(wv - _scrollBarMargin.left - _scrollBarMargin.right);
            _hzScrollBar->setX(_scrollBarMargin.left);
        }
    }
    if (_vtScrollBar != nullptr)
    {
        if (!_displayOnLeft)
            _vtScrollBar->setX(wv - _vtScrollBar->getWidth());
        if (_hzScrollBar != nullptr)
            _vtScrollBar->setHeight(hv - _hzScrollBar->getHeight() - _scrollBarMargin.top - _scrollBarMargin.bottom);
        else
            _vtScrollBar->setHeight(hv - _scrollBarMargin.top - _scrollBarMargin.bottom);
        _vtScrollBar->setY(_scrollBarMargin.top);
    }

    _viewSize.x = wv;
    _viewSize.y = hv;
    if (_hzScrollBar != nullptr && !_hScrollNone)
        _viewSize.y -= _hzScrollBar->getHeight();
    if (_vtScrollBar != nullptr && !_vScrollNone)
        _viewSize.x -= _vtScrollBar->getWidth();
    _viewSize.x -= (_owner->_margin.left + _owner->_margin.right);
    _viewSize.y -= (_owner->_margin.top + _owner->_margin.bottom);

    _viewSize.x = MAX(1.0f, _viewSize.x);
    _viewSize.y = MAX(1.0f, _viewSize.y);
    _pageSize = _viewSize;

    adjustMaskContainer();
    handleSizeChanged();
}

void ScrollPane::setContentSize(float wv, float hv)
{
    if (_contentSize.x == wv && _contentSize.y == hv)
        return;

    _contentSize.x = wv;
    _contentSize.y = hv;
    handleSizeChanged();
}

void ScrollPane::changeContentSizeOnScrolling(float deltaWidth, float deltaHeight, float deltaPosX, float deltaPosY)
{
    bool isRightmost = _xPos == _overlapSize.x;
    bool isBottom = _yPos == _overlapSize.y;

    _contentSize.x += deltaWidth;
    _contentSize.y += deltaHeight;
    handleSizeChanged();

    if (_tweening == 1)
    {
        if (deltaWidth != 0 && isRightmost && _tweenChange.x < 0)
        {
            _xPos = _overlapSize.x;
            _tweenChange.x = -_xPos - _tweenStart.x;
        }

        if (deltaHeight != 0 && isBottom && _tweenChange.y < 0)
        {
            _yPos = _overlapSize.y;
            _tweenChange.y = -_yPos - _tweenStart.y;
        }
    }
    else if (_tweening == 2)
    {
        if (deltaPosX != 0)
        {
            _container->setX(_container->getX() - deltaPosX);
            _tweenStart.x -= deltaPosX;
            _xPos = -_container->getX();
        }
        if (deltaPosY != 0)
        {
            _container->setY(_container->getY() - deltaPosY);
            _tweenStart.y -= deltaPosY;
            _yPos = -_container->getY();
        }
    }
    else if (_isMouseMoved)
    {
        if (deltaPosX != 0)
        {
            _container->setX(_container->getX() - deltaPosX);
            _containerPos.x -= deltaPosX;
            _xPos = -_container->getX();
        }
        if (deltaPosY != 0)
        {
            _container->setY(_container->getY() - deltaPosY);
            _containerPos.y -= deltaPosY;
            _yPos = -_container->getY();
        }
    }
    else
    {
        if (deltaWidth != 0 && isRightmost)
        {
            _xPos = _overlapSize.x;
            _container->setX(_container->getX() - _xPos);
        }

        if (deltaHeight != 0 && isBottom)
        {
            _yPos = _overlapSize.y;
            _container->setY(_container->getY() - _yPos);
        }
    }

    if (_pageMode)
        updatePageController();
}

void ScrollPane::handleSizeChanged()
{
    if (_displayInDemand)
    {
        if (_vtScrollBar != nullptr)
        {
            if (_contentSize.y <= _viewSize.y)
            {
                if (!_vScrollNone)
                {
                    _vScrollNone = true;
                    _viewSize.x += _vtScrollBar->getWidth();
                }
            }
            else
            {
                if (_vScrollNone)
                {
                    _vScrollNone = false;
                    _viewSize.x -= _vtScrollBar->getWidth();
                }
            }
        }
        if (_hzScrollBar != nullptr)
        {
            if (_contentSize.x <= _viewSize.x)
            {
                if (!_hScrollNone)
                {
                    _hScrollNone = true;
                    _viewSize.y += _hzScrollBar->getHeight();
                }
            }
            else
            {
                if (_hScrollNone)
                {
                    _hScrollNone = false;
                    _viewSize.y -= _hzScrollBar->getHeight();
                }
            }
        }
    }

    if (_vtScrollBar != nullptr)
    {
        if (_viewSize.y < _vtScrollBar->getMinSize())
            _vtScrollBar->setVisible(false);
        else
        {
            _vtScrollBar->setVisible(_scrollBarVisible && !_vScrollNone);
            if (_contentSize.y == 0)
                _vtScrollBar->setDisplayPerc(0);
            else
                _vtScrollBar->setDisplayPerc(MIN(1.0f, _viewSize.y / _contentSize.y));
        }
    }
    if (_hzScrollBar != nullptr)
    {
        if (_viewSize.x < _hzScrollBar->getMinSize())
            _hzScrollBar->setVisible(false);
        else
        {
            _hzScrollBar->setVisible(_scrollBarVisible && !_hScrollNone);
            if (_contentSize.x == 0)
                _hzScrollBar->setDisplayPerc(0);
            else
                _hzScrollBar->setDisplayPerc(MIN(1.0f, _viewSize.x / _contentSize.x));
        }
    }

    _maskContainer->setSize(_viewSize.x, _viewSize.y);
    if (!_maskDisabled)
        _maskContainer->setClipRect(VRectanglef(0, 0, _viewSize.x, _viewSize.y));

    if (_vtScrollBar)
        _vtScrollBar->handlePositionChanged();
    if (_hzScrollBar)
        _hzScrollBar->handlePositionChanged();
    if (_header)
        _header->handlePositionChanged();
    if (_footer)
        _footer->handlePositionChanged();

    if (_scrollType == ScrollType::HORIZONTAL || _scrollType == ScrollType::BOTH)
        _overlapSize.x = ceil(MAX(0.0f, _contentSize.x - _viewSize.x));
    else
        _overlapSize.x = 0;
    if (_scrollType == ScrollType::VERTICAL || _scrollType == ScrollType::BOTH)
        _overlapSize.y = ceil(MAX(0.0f, _contentSize.y - _viewSize.y));
    else
        _overlapSize.y = 0;


    _xPos = hkvMath::clamp(_xPos, 0.0f, _overlapSize.x);
    _yPos = hkvMath::clamp(_yPos, 0.0f, _overlapSize.y);
    float max = sp_getField(_overlapSize, _refreshBarAxis);
    if (max == 0)
        max = MAX(sp_getField(_contentSize, _refreshBarAxis) + _footerLockedSize - sp_getField(_viewSize, _refreshBarAxis), 0.0f);
    else
        max += _footerLockedSize;
    if (_refreshBarAxis == 0)
        _container->setPosition(hkvMath::clamp(_container->getX(), -max, (float)_headerLockedSize),
            hkvMath::clamp(_container->getY(), -_overlapSize.y, 0.0f));
    else
        _container->setPosition(hkvMath::clamp(_container->getX(), -_overlapSize.x, 0.0f),
            hkvMath::clamp(_container->getY(), -max, (float)_headerLockedSize));

    if (_header != nullptr)
    {
        if (_refreshBarAxis == 0)
            _header->setHeight(_viewSize.y);
        else
            _header->setWidth(_viewSize.x);
    }

    if (_footer != nullptr)
    {
        if (_refreshBarAxis == 0)
            _footer->setHeight(_viewSize.y);
        else
            _footer->setWidth(_viewSize.x);
    }

    syncScrollBar();
    checkRefreshBar();
    if (_pageMode)
        updatePageController();
}

void ScrollPane::posChanged(bool ani)
{
    if (_aniFlag == 0)
        _aniFlag = ani ? 1 : -1;
    else if (_aniFlag == 1 && !ani)
        _aniFlag = -1;

    _needRefresh = true;
    _owner->scheduleOnce(SCHEDULE_SELECTOR(GComponent::refreshScrollPane));
}

void ScrollPane::refresh()
{
    _owner->unSchedule(SCHEDULE_SELECTOR(GComponent::refreshScrollPane));
    _needRefresh = false;

    if (_pageMode || _snapToItem)
    {
        hkvVec2 pos(-_xPos, -_yPos);
        alignPosition(pos, false);
        _xPos = -pos.x;
        _yPos = -pos.y;
    }

    refresh2();

    _owner->dispatchEvent(UIEventType::Scroll);
    if (_needRefresh) //pos may change in onScroll
    {
        _needRefresh = false;
        _owner->unSchedule(SCHEDULE_SELECTOR(GComponent::refreshScrollPane));

        refresh2();
    }

    syncScrollBar();
    _aniFlag = 0;
}

void ScrollPane::refresh2()
{
    if (_aniFlag == 1 && !_isMouseMoved)
    {
        hkvVec2 pos;

        if (_overlapSize.x > 0)
            pos.x = -(int)_xPos;
        else
        {
            if (_container->getX() != 0)
                _container->setX(0);
            pos.x = 0;
        }
        if (_overlapSize.y > 0)
            pos.y = -(int)_yPos;
        else
        {
            if (_container->getY() != 0)
                _container->setY(0);
            pos.y = 0;
        }

        if (pos.x != _container->getX() || pos.y != _container->getY())
        {
            _tweening = 1;
            _tweenTime.setZero();
            _tweenDuration.set(TWEEN_TIME_GO, TWEEN_TIME_GO);
            _tweenStart = _container->getPosition();
            _tweenChange = pos - _tweenStart;
            _owner->schedule(SCHEDULE_SELECTOR(GComponent::scrollPaneUpdate), 0);
        }
        else if (_tweening != 0)
            killTween();
    }
    else
    {
        if (_tweening != 0)
            killTween();

        _container->setPosition((int)-_xPos, (int)-_yPos);

        loopCheckingCurrent();
    }

    if (_pageMode)
        updatePageController();
}

void ScrollPane::syncScrollBar(bool end)
{
    if (_vtScrollBar != nullptr)
    {
        _vtScrollBar->setScrollPerc(_overlapSize.y == 0 ? 0 : hkvMath::clamp(-_container->getY(), 0.0f, _overlapSize.y) / _overlapSize.y);
        if (_scrollBarDisplayAuto)
            showScrollBar(!end);
    }
    if (_hzScrollBar != nullptr)
    {
        _hzScrollBar->setScrollPerc(_overlapSize.x == 0 ? 0 : hkvMath::clamp(-_container->getX(), 0.0f, _overlapSize.x) / _overlapSize.x);
        if (_scrollBarDisplayAuto)
            showScrollBar(!end);
    }
}

void ScrollPane::showScrollBar(bool show)
{
    _scrollBarVisible = (bool)show && _viewSize.x > 0 && _viewSize.y > 0;

    if (show)
    {
        onShowScrollBar();
        _owner->unSchedule(SCHEDULE_SELECTOR(GComponent::onShowScrollBar));
    }
    else
        _owner->schedule(SCHEDULE_SELECTOR(GComponent::onShowScrollBar), 0, 0, 0.5f);
}

void ScrollPane::onShowScrollBar()
{
    if (_vtScrollBar != nullptr)
        _vtScrollBar->setVisible(_scrollBarVisible && !_vScrollNone);
    if (_hzScrollBar != nullptr)
        _hzScrollBar->setVisible(_scrollBarVisible && !_hScrollNone);
}

float ScrollPane::getLoopPartSize(float division, int axis)
{
    return (sp_getField(_contentSize, axis) + (axis == 0 ? ((GList*)_owner)->getColumnGap() : ((GList*)_owner)->getLineGap())) / division;
}

bool ScrollPane::loopCheckingCurrent()
{
    bool changed = false;
    if (_loop == 1 && _overlapSize.x > 0)
    {
        if (_xPos < 0.001f)
        {
            _xPos += getLoopPartSize(2, 0);
            changed = true;
        }
        else if (_xPos >= _overlapSize.x)
        {
            _xPos -= getLoopPartSize(2, 0);
            changed = true;
        }
    }
    else if (_loop == 2 && _overlapSize.y > 0)
    {
        if (_yPos < 0.001f)
        {
            _yPos += getLoopPartSize(2, 1);
            changed = true;
        }
        else if (_yPos >= _overlapSize.y)
        {
            _yPos -= getLoopPartSize(2, 1);
            changed = true;
        }
    }

    if (changed)
        _container->setPosition((int)-_xPos, (int)-_yPos);

    return changed;
}

void ScrollPane::loopCheckingTarget(hkvVec2 & endPos)
{
    if (_loop == 1)
        loopCheckingTarget(endPos, 0);

    if (_loop == 2)
        loopCheckingTarget(endPos, 1);
}

void ScrollPane::loopCheckingTarget(hkvVec2 & endPos, int axis)
{
    if (sp_getField(endPos, axis) > 0)
    {
        float halfSize = getLoopPartSize(2, axis);
        float tmp = sp_getField(_tweenStart, axis) - halfSize;
        if (tmp <= 0 && tmp >= -sp_getField(_overlapSize, axis))
        {
            sp_incField(endPos, axis, -halfSize);
            sp_setField(_tweenStart, axis, tmp);
        }
    }
    else if (sp_getField(endPos, axis) < -sp_getField(_overlapSize, axis))
    {
        float halfSize = getLoopPartSize(2, axis);
        float tmp = sp_getField(_tweenStart, axis) + halfSize;
        if (tmp <= 0 && tmp >= -sp_getField(_overlapSize, axis))
        {
            sp_incField(endPos, axis, halfSize);
            sp_setField(_tweenStart, axis, tmp);
        }
    }
}

void ScrollPane::loopCheckingNewPos(float & value, int axis)
{
    float overlapSize = sp_getField(_overlapSize, axis);
    if (overlapSize == 0)
        return;

    float pos = axis == 0 ? _xPos : _yPos;
    bool changed = false;
    if (value < 0.001f)
    {
        value += getLoopPartSize(2, axis);
        if (value > pos)
        {
            float v = getLoopPartSize(6, axis);
            v = ceil((value - pos) / v) * v;
            pos = hkvMath::clamp(pos + v, 0.0f, overlapSize);
            changed = true;
        }
    }
    else if (value >= overlapSize)
    {
        value -= getLoopPartSize(2, axis);
        if (value < pos)
        {
            float v = getLoopPartSize(6, axis);
            v = ceil((pos - value) / v) * v;
            pos = hkvMath::clamp(pos - v, 0.0f, overlapSize);
            changed = true;
        }
    }

    if (changed)
    {
        if (axis == 0)
            _container->setX(-(int)pos);
        else
            _container->setY(-(int)pos);
    }
}

void ScrollPane::alignPosition(hkvVec2 & pos, bool inertialScrolling)
{
    if (_pageMode)
    {
        pos.x = alignByPage(pos.x, 0, inertialScrolling);
        pos.y = alignByPage(pos.y, 1, inertialScrolling);
    }
    else if (_snapToItem)
    {
        hkvVec2 tmp = _owner->getSnappingPosition(-pos);
        if (pos.x < 0 && pos.x > -_overlapSize.x)
            pos.x = -tmp.x;
        if (pos.y < 0 && pos.y > -_overlapSize.y)
            pos.y = -tmp.y;
    }
}

float ScrollPane::alignByPage(float pos, int axis, bool inertialScrolling)
{
    int page;
    float pageSize = sp_getField(_pageSize, axis);
    float overlapSize = sp_getField(_overlapSize, axis);
    float contentSize = sp_getField(_contentSize, axis);

    if (pos > 0)
        page = 0;
    else if (pos < -overlapSize)
        page = (int)ceil(contentSize / pageSize) - 1;
    else
    {
        page = (int)floor(-pos / pageSize);
        float change = inertialScrolling ? (pos - sp_getField(_containerPos, axis)) : (pos - sp_getField(_container->getPosition(), axis));
        float testPageSize = MIN(pageSize, contentSize - (page + 1) * pageSize);
        float delta = -pos - page * pageSize;

        if (std::abs(change) > pageSize)
        {
            if (delta > testPageSize * 0.5f)
                page++;
        }
        else
        {
            if (delta > testPageSize * (change < 0 ? 0.3f : 0.7f))
                page++;
        }

        pos = -page * pageSize;
        if (pos < -overlapSize)
            pos = -overlapSize;
    }

    if (inertialScrolling)
    {
        float oldPos = sp_getField(_tweenStart, axis);
        int oldPage;
        if (oldPos > 0)
            oldPage = 0;
        else if (oldPos < -overlapSize)
            oldPage = (int)ceil(contentSize / pageSize) - 1;
        else
            oldPage = (int)floor(-oldPos / pageSize);
        int startPage = (int)floor(-sp_getField(_containerPos, axis) / pageSize);
        if (abs(page - startPage) > 1 && abs(oldPage - startPage) <= 1)
        {
            if (page > startPage)
                page = startPage + 1;
            else
                page = startPage - 1;
            pos = -page * pageSize;
        }
    }

    return pos;
}

hkvVec2 ScrollPane::updateTargetAndDuration(const hkvVec2 & orignPos)
{
    hkvVec2 ret(0, 0);
    ret.x = updateTargetAndDuration(orignPos.x, 0);
    ret.y = updateTargetAndDuration(orignPos.y, 1);
    return ret;
}

float ScrollPane::updateTargetAndDuration(float pos, int axis)
{
    float v = sp_getField(_velocity, axis);
    float duration = 0;

    if (pos > 0)
        pos = 0;
    else if (pos < -sp_getField(_overlapSize, axis))
        pos = -sp_getField(_overlapSize, axis);
    else
    {
        float v2 = std::abs(v) * _velocityScale;
        float ratio = 0;
#ifdef CC_PLATFORM_PC
        if (v2 > 500)
            ratio = pow((v2 - 500) / 500, 2);
#else
        const hkvVec2& winSize = StageInst->getSize();
        v2 *= 1136.0f / MAX(winSize.x, winSize.y);

        if (_pageMode)
        {
            if (v2 > 500)
                ratio = pow((v2 - 500) / 500, 2);
        }
        else
        {
            if (v2 > 1000)
                ratio = pow((v2 - 1000) / 1000, 2);
        }
#endif

        if (ratio != 0)
        {
            if (ratio > 1)
                ratio = 1;

            v2 *= ratio;
            v *= ratio;
            sp_setField(_velocity, axis, v);

            duration = hkvMath::log(60 / v2) / hkvMath::log(_decelerationRate) / 60;
            float change = (int)(v * duration * 0.4f);
            pos += change;
        }
    }

    if (duration < TWEEN_TIME_DEFAULT)
        duration = TWEEN_TIME_DEFAULT;
    sp_setField(_tweenDuration, axis, duration);

    return pos;
}

void ScrollPane::fixDuration(int axis, float oldChange)
{
    float tweenChange = sp_getField(_tweenChange, axis);
    if (tweenChange == 0 || std::abs(tweenChange) >= std::abs(oldChange))
        return;

    float newDuration = std::abs(tweenChange / oldChange) * sp_getField(_tweenDuration, axis);
    if (newDuration < TWEEN_TIME_DEFAULT)
        newDuration = TWEEN_TIME_DEFAULT;

    sp_setField(_tweenDuration, axis, newDuration);
}

void ScrollPane::killTween()
{
    if (_tweening == 1)
    {
        hkvVec2 t = _tweenStart + _tweenChange;
        _container->setPosition(t.x, t.y);
        _owner->dispatchEvent(UIEventType::Scroll);
    }

    _tweening = 0;
    _owner->unSchedule(SCHEDULE_SELECTOR(GComponent::scrollPaneUpdate));
    _owner->dispatchEvent(UIEventType::ScrollEnd);
}

void ScrollPane::checkRefreshBar()
{
    if (_header == nullptr && _footer == nullptr)
        return;

    float pos = sp_getField(_container->getPosition(), _refreshBarAxis);
    if (_header != nullptr)
    {
        if (pos > 0)
        {
            if (_header->displayObject()->getParent() == nullptr)
                _maskContainer->addChildAt(_header->displayObject(), 0);
            hkvVec2 vec;

            vec = _header->getSize();
            sp_setField(vec, _refreshBarAxis, pos);
            _header->setSize(vec.x, vec.y);
        }
        else
        {
            if (_header->displayObject()->getParent() != nullptr)
                _maskContainer->removeChild(_header->displayObject());
        }
    }

    if (_footer != nullptr)
    {
        float max = sp_getField(_overlapSize, _refreshBarAxis);
        if (pos < -max || (max == 0 && _footerLockedSize > 0))
        {
            if (_footer->displayObject()->getParent() == nullptr)
                _maskContainer->addChildAt(_footer->displayObject(), 0);

            hkvVec2 vec;

            vec = _footer->getPosition();
            if (max > 0)
                sp_setField(vec, _refreshBarAxis, pos + sp_getField(_contentSize, _refreshBarAxis));
            else
                sp_setField(vec, _refreshBarAxis, MAX(MIN(pos + sp_getField(_viewSize, _refreshBarAxis),
                    sp_getField(_viewSize, _refreshBarAxis) - _footerLockedSize),
                    sp_getField(_viewSize, _refreshBarAxis) - sp_getField(_contentSize, _refreshBarAxis)));
            _footer->setPosition(vec.x, vec.y);

            vec = _footer->getSize();
            if (max > 0)
                sp_setField(vec, _refreshBarAxis, -max - pos);
            else
                sp_setField(vec, _refreshBarAxis, sp_getField(_viewSize, _refreshBarAxis) - sp_getField(_footer->getPosition(), _refreshBarAxis));
            _footer->setSize(vec.x, vec.y);
        }
        else
        {
            if (_footer->displayObject()->getParent() != nullptr)
                _maskContainer->removeChild(_footer->displayObject());
        }
    }
}

void ScrollPane::tweenUpdate(float dt)
{
    float nx = runTween(0, dt);
    float ny = runTween(1, dt);

    _container->setPosition(nx, ny);

    if (_tweening == 2)
    {
        if (_overlapSize.x > 0)
            _xPos = hkvMath::clamp(-nx, 0.0f, _overlapSize.x);
        if (_overlapSize.y > 0)
            _yPos = hkvMath::clamp(-ny, 0.0f, _overlapSize.y);

        if (_pageMode)
            updatePageController();
    }

    if (_tweenChange.x == 0 && _tweenChange.y == 0)
    {
        _tweening = 0;
        _owner->unSchedule(SCHEDULE_SELECTOR(GComponent::scrollPaneUpdate));

        loopCheckingCurrent();

        syncScrollBar(true);
        checkRefreshBar();
        _owner->dispatchEvent(UIEventType::Scroll);
        _owner->dispatchEvent(UIEventType::ScrollEnd);
    }
    else
    {
        syncScrollBar(false);
        checkRefreshBar();
        _owner->dispatchEvent(UIEventType::Scroll);
    }
}

float ScrollPane::runTween(int axis, float dt)
{
    float newValue;
    if (sp_getField(_tweenChange, axis) != 0)
    {
        sp_incField(_tweenTime, axis, dt);
        if (sp_getField(_tweenTime, axis) >= sp_getField(_tweenDuration, axis))
        {
            newValue = sp_getField(_tweenStart, axis) + sp_getField(_tweenChange, axis);
            sp_setField(_tweenChange, axis, 0);
        }
        else
        {
            float ratio = sp_EaseFunc(sp_getField(_tweenTime, axis), sp_getField(_tweenDuration, axis));
            newValue = sp_getField(_tweenStart, axis) + (int)(sp_getField(_tweenChange, axis) * ratio);
        }

        float threshold1 = 0;
        float threshold2 = -sp_getField(_overlapSize, axis);
        if (_headerLockedSize > 0 && _refreshBarAxis == axis)
            threshold1 = _headerLockedSize;
        if (_footerLockedSize > 0 && _refreshBarAxis == axis)
        {
            float max = sp_getField(_overlapSize, _refreshBarAxis);
            if (max == 0)
                max = MAX(sp_getField(_contentSize, _refreshBarAxis) + _footerLockedSize - sp_getField(_viewSize, _refreshBarAxis), 0.0f);
            else
                max += _footerLockedSize;
            threshold2 = -max;
        }

        if (_tweening == 2 && _bouncebackEffect)
        {
            if ((newValue > 20 + threshold1 && sp_getField(_tweenChange, axis) > 0)
                || (newValue > threshold1 && sp_getField(_tweenChange, axis) == 0))
            {
                sp_setField(_tweenTime, axis, 0);
                sp_setField(_tweenDuration, axis, TWEEN_TIME_DEFAULT);
                sp_setField(_tweenChange, axis, -newValue + threshold1);
                sp_setField(_tweenStart, axis, newValue);
            }
            else if ((newValue < threshold2 - 20 && sp_getField(_tweenChange, axis) < 0)
                || (newValue < threshold2 && sp_getField(_tweenChange, axis) == 0))
            {
                sp_setField(_tweenTime, axis, 0);
                sp_setField(_tweenDuration, axis, TWEEN_TIME_DEFAULT);
                sp_setField(_tweenChange, axis, threshold2 - newValue);
                sp_setField(_tweenStart, axis, newValue);
            }
        }
        else
        {
            if (newValue > threshold1)
            {
                newValue = threshold1;
                sp_setField(_tweenChange, axis, 0);
            }
            else if (newValue < threshold2)
            {
                newValue = threshold2;
                sp_setField(_tweenChange, axis, 0);
            }
        }
    }
    else
        newValue = sp_getField(_container->getPosition(), axis);

    return newValue;
}

void ScrollPane::onTouchBegin(EventContext * context)
{
    if (!_touchEffect)
        return;

    context->captureTouch();
    InputEvent* evt = context->getInput();
    hkvVec2 pt = _owner->globalToLocal(evt->getPosition());

    if (_tweening != 0)
    {
        killTween();
        StageInst->cancelClick(evt->getTouchId());

        _isMouseMoved = true;
    }
    else
        _isMouseMoved = false;

    _containerPos = _container->getPosition();
    _beginTouchPos = _lastTouchPos = pt;
    _lastTouchGlobalPos = evt->getPosition();
    _isHoldAreaDone = false;
    _velocity.setZero();
    _velocityScale = 1;
    _lastMoveTime = clock();
}

void ScrollPane::onTouchMove(EventContext * context)
{
    if (!_touchEffect)
        return;

    if ((_draggingPane != nullptr && _draggingPane != this) || GObject::getDraggingObject() != nullptr)
        return;

    InputEvent* evt = context->getInput();
    hkvVec2 pt = _owner->globalToLocal(evt->getPosition());

    int sensitivity;
#ifdef CC_PLATFORM_PC
    sensitivity = 8;
#else
    sensitivity = UIConfig::touchScrollSensitivity;
#endif

    float diff;
    bool sv = false, sh = false;

    if (_scrollType == ScrollType::VERTICAL)
    {
        if (!_isHoldAreaDone)
        {
            _gestureFlag |= 1;

            diff = std::abs(_beginTouchPos.y - pt.y);
            if (diff < sensitivity)
                return;

            if ((_gestureFlag & 2) != 0)
            {
                float diff2 = std::abs(_beginTouchPos.x - pt.x);
                if (diff < diff2)
                    return;
            }
        }

        sv = true;
    }
    else if (_scrollType == ScrollType::HORIZONTAL)
    {
        if (!_isHoldAreaDone)
        {
            _gestureFlag |= 2;

            diff = std::abs(_beginTouchPos.x - pt.x);
            if (diff < sensitivity)
                return;

            if ((_gestureFlag & 1) != 0)
            {
                float diff2 = std::abs(_beginTouchPos.y - pt.y);
                if (diff < diff2)
                    return;
            }
        }

        sh = true;
    }
    else
    {
        _gestureFlag = 3;

        if (!_isHoldAreaDone)
        {
            diff = std::abs(_beginTouchPos.y - pt.y);
            if (diff < sensitivity)
            {
                diff = std::abs(_beginTouchPos.x - pt.x);
                if (diff < sensitivity)
                    return;
            }
        }

        sv = sh = true;
    }

    hkvVec2 newPos = _containerPos + pt - _beginTouchPos;
    newPos.x = (int)newPos.x;
    newPos.y = (int)newPos.y;

    if (sv)
    {
        if (newPos.y > 0)
        {
            if (!_bouncebackEffect)
                _container->setY(0);
            else if (_header != nullptr && _header->maxSize.y != 0)
                _container->setY(((int)MIN(newPos.y * 0.5f, _header->maxSize.y)));
            else
                _container->setY(((int)MIN(newPos.y * 0.5f, _viewSize.y * PULL_RATIO)));
        }
        else if (newPos.y < -_overlapSize.y)
        {
            if (!_bouncebackEffect)
                _container->setY(-_overlapSize.y);
            else if (_footer != nullptr && _footer->maxSize.y > 0)
                _container->setY(((int)MAX((newPos.y + _overlapSize.y) * 0.5f, -_footer->maxSize.y) - _overlapSize.y));
            else
                _container->setY(((int)MAX((newPos.y + _overlapSize.y) * 0.5f, -_viewSize.y * PULL_RATIO) - _overlapSize.y));
        }
        else
            _container->setY(newPos.y);
    }

    if (sh)
    {
        if (newPos.x > 0)
        {
            if (!_bouncebackEffect)
                _container->setX(0);
            else if (_header != nullptr && _header->maxSize.x != 0)
                _container->setX((int)MIN(newPos.x * 0.5f, _header->maxSize.x));
            else
                _container->setX((int)MIN(newPos.x * 0.5f, _viewSize.x * PULL_RATIO));
        }
        else if (newPos.x < 0 - _overlapSize.x)
        {
            if (!_bouncebackEffect)
                _container->setX(-_overlapSize.x);
            else if (_footer != nullptr && _footer->maxSize.x > 0)
                _container->setX((int)MAX((newPos.x + _overlapSize.x) * 0.5f, -_footer->maxSize.x) - _overlapSize.x);
            else
                _container->setX((int)MAX((newPos.x + _overlapSize.x) * 0.5f, -_viewSize.x * PULL_RATIO) - _overlapSize.x);
        }
        else
            _container->setX(newPos.x);
    }

    auto deltaTime = Vision::GetTimer()->GetTimeDifference();
    float elapsed = (float)((clock() - _lastMoveTime) / (double)CLOCKS_PER_SEC);
    elapsed = elapsed * 60 - 1;
    if (elapsed > 1)
        _velocity = _velocity * pow(0.833f, elapsed);
    hkvVec2 deltaPosition = pt - _lastTouchPos;
    if (!sh)
        deltaPosition.x = 0;
    if (!sv)
        deltaPosition.y = 0;
    _velocity = VLerp<hkvVec2>()(_velocity, deltaPosition / deltaTime, deltaTime * 10);

    hkvVec2 deltaGlobalPosition = _lastTouchGlobalPos - evt->getPosition();
    if (deltaPosition.x != 0)
        _velocityScale = std::abs(deltaGlobalPosition.x / deltaPosition.x);
    else if (deltaPosition.y != 0)
        _velocityScale = std::abs(deltaGlobalPosition.y / deltaPosition.y);

    _lastTouchPos = pt;
    _lastTouchGlobalPos = evt->getPosition();
    _lastMoveTime = clock();

    if (_overlapSize.x > 0)
        _xPos = hkvMath::clamp(-_container->getX(), 0.0f, _overlapSize.x);
    if (_overlapSize.y > 0)
        _yPos = hkvMath::clamp(-_container->getY(), 0.0f, _overlapSize.y);

    if (_loop != 0)
    {
        newPos = _container->getPosition();
        if (loopCheckingCurrent())
            _containerPos += _container->getPosition() - newPos;
    }

    _draggingPane = this;
    _isHoldAreaDone = true;
    _isMouseMoved = true;

    syncScrollBar();
    checkRefreshBar();
    if (_pageMode)
        updatePageController();
    _owner->dispatchEvent(UIEventType::Scroll);
}

void ScrollPane::onTouchEnd(EventContext * context)
{
    if (_draggingPane == this)
        _draggingPane = nullptr;

    _gestureFlag = 0;

    if (!_isMouseMoved || !_touchEffect)
    {
        _isMouseMoved = false;
        return;
    }

    _isMouseMoved = false;
    _tweenStart = _container->getPosition();

    hkvVec2 endPos = _tweenStart;
    bool flag = false;
    if (_container->getX() > 0)
    {
        endPos.x = 0;
        flag = true;
    }
    else if (_container->getX() < -_overlapSize.x)
    {
        endPos.x = -_overlapSize.x;
        flag = true;
    }
    if (_container->getY() > 0)
    {
        endPos.y = 0;
        flag = true;
    }
    else if (_container->getY() < -_overlapSize.y)
    {
        endPos.y = -_overlapSize.y;
        flag = true;
    }

    if (flag)
    {
        _tweenChange = endPos - _tweenStart;
        if (_tweenChange.x < -UIConfig::touchDragSensitivity || _tweenChange.y < -UIConfig::touchDragSensitivity)
            _owner->dispatchEvent(UIEventType::PullDownRelease);
        else if (_tweenChange.x > UIConfig::touchDragSensitivity || _tweenChange.y > UIConfig::touchDragSensitivity)
            _owner->dispatchEvent(UIEventType::PullUpRelease);

        if (_headerLockedSize > 0 && sp_getField(endPos, _refreshBarAxis) == 0)
        {
            sp_setField(endPos, _refreshBarAxis, _headerLockedSize);
            _tweenChange = endPos - _tweenStart;
        }
        else if (_footerLockedSize > 0 && sp_getField(endPos, _refreshBarAxis) == -sp_getField(_overlapSize, _refreshBarAxis))
        {
            float max = sp_getField(_overlapSize, _refreshBarAxis);
            if (max == 0)
                max = MAX(sp_getField(_contentSize, _refreshBarAxis) + _footerLockedSize - sp_getField(_viewSize, _refreshBarAxis), 0.0f);
            else
                max += _footerLockedSize;
            sp_setField(endPos, _refreshBarAxis, -max);
            _tweenChange = endPos - _tweenStart;
        }

        _tweenDuration.set(TWEEN_TIME_DEFAULT, TWEEN_TIME_DEFAULT);
    }
    else
    {
        if (!_inertiaDisabled)
        {
            float elapsed = (float)((clock() - _lastMoveTime) / (double)CLOCKS_PER_SEC);
            elapsed = elapsed * 60 - 1;
            if (elapsed > 1)
                _velocity = _velocity * pow(0.833f, elapsed);

            endPos = updateTargetAndDuration(_tweenStart);
        }
        else
            _tweenDuration.set(TWEEN_TIME_DEFAULT, TWEEN_TIME_DEFAULT);
        hkvVec2 oldChange = endPos - _tweenStart;

        loopCheckingTarget(endPos);
        if (_pageMode || _snapToItem)
            alignPosition(endPos, true);

        _tweenChange = endPos - _tweenStart;
        if (_tweenChange.x == 0 && _tweenChange.y == 0)
            return;

        if (_pageMode || _snapToItem)
        {
            fixDuration(0, oldChange.x);
            fixDuration(1, oldChange.y);
        }
    }

    _tweening = 2;
    _tweenTime.setZero();
    _owner->schedule(SCHEDULE_SELECTOR(GComponent::scrollPaneUpdate), 0);
}

void ScrollPane::onMouseWheel(EventContext * context)
{
    if (!_mouseWheelEnabled)
        return;

    InputEvent* evt = context->getInput();
    int delta = evt->getMouseWheelDelta();
    delta = delta > 0 ? 1 : -1;
    if (_overlapSize.x > 0 && _overlapSize.y == 0)
    {
        if (_pageMode)
            setPosX(_xPos + _pageSize.x * delta, false);
        else
            setPosX(_xPos + _mouseWheelStep * delta, false);
    }
    else
    {
        if (_pageMode)
            setPosY(_yPos + _pageSize.y * delta, false);
        else
            setPosY(_yPos + _mouseWheelStep * delta, false);
    }
}

void ScrollPane::onRollOver(EventContext * context)
{
    showScrollBar(true);
}

void ScrollPane::onRollOut(EventContext * context)
{
    showScrollBar(false);
}

NS_FGUI_END