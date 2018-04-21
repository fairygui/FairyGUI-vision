#include "GRoot.h"
#include "GGraph.h"
#include "UIPackage.h"
#include "UIConfig.h"
#include "Window.h"
#include "DragDropManager.h"

NS_FGUI_BEGIN

GRoot::GRoot() :
    _modalLayer(nullptr),
    _modalWaitPane(nullptr),
    _tooltipWin(nullptr),
    _defaultTooltipWin(nullptr),
    _dragDropManager(nullptr),
    _scaleFactor(1)
{
}

GRoot::~GRoot()
{
    CC_SAFE_RELEASE(_modalWaitPane);
    CC_SAFE_RELEASE(_defaultTooltipWin);
    CC_SAFE_RELEASE(_modalLayer);

    CC_SAFE_DELETE(_dragDropManager);
}

void GRoot::handleInit()
{
    GComponent::handleInit();

    setOpaque(false);
    setSize(StageInst->getWidth(), StageInst->getHeight());

    StageInst->addListener(UIEventType::SizeChange, CALLBACK_1(GRoot::onWindowSizeChanged, this));
    StageInst->addListener(UIEventType::StageTouchBegin, CALLBACK_1(GRoot::onTouchBegin, this));

    _dragDropManager = new DragDropManager(this);
}

void GRoot::setContentScaleFactor(int designResolutionX, int designResolutionY, ScreenMatchMode screenMatchMode)
{
    if (designResolutionX == 0 || designResolutionY == 0)
        return;

    bool ignoreOrientation = false;

    int dx = designResolutionX;
    int dy = designResolutionY;
    int screenWidth = StageInst->getWidth();
    int screenHeight = StageInst->getHeight();
    if (!ignoreOrientation && (screenWidth > screenHeight && dx < dy || screenWidth < screenHeight && dx > dy))
    {
        //scale should not change when orientation change
        int tmp = dx;
        dx = dy;
        dy = tmp;
    }

    if (screenMatchMode == ScreenMatchMode::MatchWidthOrHeight)
    {
        float s1 = (float)screenWidth / dx;
        float s2 = (float)screenHeight / dy;
        _scaleFactor = MIN(s1, s2);
    }
    else if (screenMatchMode == ScreenMatchMode::MatchWidth)
        _scaleFactor = (float)screenWidth / dx;
    else
        _scaleFactor = (float)screenHeight / dy;

    onWindowSizeChanged(nullptr);
}

void GRoot::showWindow(Window * win)
{
    addChild(win);
    adjustModalLayer();
}

void GRoot::hideWindow(Window * win)
{
    win->hide();
}

void GRoot::hideWindowImmediately(Window * win)
{
    if (win->getParent() == this)
        removeChild(win);

    adjustModalLayer();
}

void GRoot::bringToFront(Window * win)
{
    int cnt = numChildren();
    int i;
    if (_modalLayer->getParent() != nullptr && !win->isModal())
        i = getChildIndex(_modalLayer) - 1;
    else
        i = cnt - 1;

    for (; i >= 0; i--)
    {
        GObject* g = getChildAt(i);
        if (g == win)
            return;
        if (dynamic_cast<Window*>(g))
            break;
    }

    if (i >= 0)
        setChildIndex(win, i);
}

void GRoot::closeAllExceptModals()
{
    Vector<GObject*> map(_children);

    for (const auto&child : map)
    {
        if (dynamic_cast<Window*>(child) && !((Window*)child)->isModal())
            hideWindowImmediately((Window*)child);
    }
}

void GRoot::closeAllWindows()
{
    Vector<GObject*> map(_children);

    for (const auto&child : map)
    {
        if (dynamic_cast<Window*>(child))
            hideWindowImmediately((Window*)child);
    }
}

Window * GRoot::getTopWindow()
{
    int cnt = numChildren();
    for (int i = cnt - 1; i >= 0; i--)
    {
        GObject* child = getChildAt(i);
        if (dynamic_cast<Window*>(child))
        {
            return (Window*)child;
        }
    }

    return nullptr;
}

GGraph * GRoot::getModalLayer()
{
    if (_modalLayer == nullptr)
        createModalLayer();

    return _modalLayer;
}

void GRoot::createModalLayer()
{
    _modalLayer = GGraph::create();
    _modalLayer->retain();
    _modalLayer->drawRect(getWidth(), getHeight(), 0, V_RGBA_WHITE, UIConfig::modalLayerColor);
    _modalLayer->addRelation(this, RelationType::Size);
}

void GRoot::adjustModalLayer()
{
    if (_modalLayer == nullptr)
        createModalLayer();

    int cnt = numChildren();

    if (_modalWaitPane != nullptr && _modalWaitPane->getParent() != nullptr)
        setChildIndex(_modalWaitPane, cnt - 1);

    for (int i = cnt - 1; i >= 0; i--)
    {
        GObject* child = getChildAt(i);
        if (dynamic_cast<Window*>(child) && ((Window*)child)->isModal())
        {
            if (_modalLayer->getParent() == nullptr)
                addChildAt(_modalLayer, i);
            else
                setChildIndexBefore(_modalLayer, i);
            return;
        }
    }

    if (_modalLayer->getParent() != nullptr)
        removeChild(_modalLayer);
}

bool GRoot::hasModalWindow()
{
    return _modalLayer != nullptr && _modalLayer->getParent() != nullptr;
}

void GRoot::showModalWait()
{
    getModalWaitingPane();
    if (_modalWaitPane)
        addChild(_modalWaitPane);
}

void GRoot::closeModalWait()
{
    if (_modalWaitPane != nullptr && _modalWaitPane->getParent() != nullptr)
        removeChild(_modalWaitPane);
}

GObject * GRoot::getModalWaitingPane()
{
    if (!UIConfig::globalModalWaiting.empty())
    {
        if (_modalWaitPane == nullptr)
        {
            _modalWaitPane = UIPackage::createObjectFromURL(UIConfig::globalModalWaiting);
            _modalWaitPane->setSortingOrder(INT_MAX);
            _modalWaitPane->retain();
        }

        _modalWaitPane->setSize(getWidth(), getHeight());
        _modalWaitPane->addRelation(this, RelationType::Size);

        return _modalWaitPane;
    }
    else
        return nullptr;
}

bool GRoot::isModalWaiting()
{
    return (_modalWaitPane != nullptr) && _modalWaitPane->onStage();
}

GObject* GRoot::displayObjectToGObject(DisplayObject* obj)
{
    while (obj != nullptr)
    {
        if (obj->getSpectator() != nullptr)
        {
            GObject* ret = dynamic_cast<GObject*>(obj->getSpectator());
            if (ret)
                return ret;
        }

        obj = obj->getParent();
    }
    return nullptr;
}

GObject * GRoot::getTouchTarget()
{
    return displayObjectToGObject(StageInst->getTouchTarget());
}

void GRoot::showPopup(GObject * popup)
{
    showPopup(popup, nullptr, PopupDirection::AUTO);
}

void GRoot::showPopup(GObject * popup, GObject * target, PopupDirection dir)
{
    if (!_popupStack.empty())
        hidePopup(popup);

    _popupStack.push_back(WeakPtr(popup));

    if (target != nullptr)
    {
        GObject* p = target;
        while (p != nullptr)
        {
            if (p->getParent() == this)
            {
                if (popup->getSortingOrder() < p->getSortingOrder())
                {
                    popup->setSortingOrder(p->getSortingOrder());
                }
                break;
            }
            p = p->getParent();
        }
    }

    addChild(popup);
    adjustModalLayer();

    if (dynamic_cast<Window*>(popup) && target == nullptr && dir == PopupDirection::AUTO)
        return;

    hkvVec2 pos = getPoupPosition(popup, target, dir);
    popup->setPosition(pos.x, pos.y);
}

void GRoot::togglePopup(GObject * popup)
{
    togglePopup(popup, nullptr, PopupDirection::AUTO);
}

void GRoot::togglePopup(GObject * popup, GObject * target, PopupDirection dir)
{
    if (std::find(_justClosedPopups.cbegin(), _justClosedPopups.cend(), popup) != _justClosedPopups.cend())
        return;

    showPopup(popup, target, dir);
}

void GRoot::hidePopup()
{
    hidePopup(nullptr);
}

void GRoot::hidePopup(GObject * popup)
{
    if (popup != nullptr)
    {
        auto it = std::find(_popupStack.cbegin(), _popupStack.cend(), popup);
        if (it != _popupStack.cend())
        {
            int k = (int)(it - _popupStack.cbegin());
            for (int i = (int)_popupStack.size() - 1; i >= k; i--)
            {
                closePopup(_popupStack.back().ptr<GObject>());
                _popupStack.pop_back();
            }
        }
    }
    else
    {
        for (const auto &it : _popupStack)
            closePopup(it.ptr<GObject>());
        _popupStack.clear();
    }
}

void GRoot::closePopup(GObject * target)
{
    if (target && target->getParent() != nullptr)
    {
        if (dynamic_cast<Window*>(target))
            ((Window*)target)->hide();
        else
            removeChild(target);
    }
}

void GRoot::checkPopups()
{
    _justClosedPopups.clear();
    if (!_popupStack.empty())
    {
        GObject* mc = getTouchTarget();
        bool handled = false;
        while (mc != this && mc != nullptr)
        {
            auto it = std::find(_popupStack.cbegin(), _popupStack.cend(), mc);
            if (it != _popupStack.cend())
            {
                int k = (int)(it - _popupStack.cbegin());
                for (int i = (int)_popupStack.size() - 1; i > k; i--)
                {
                    closePopup(_popupStack.back().ptr<GObject>());
                    _popupStack.pop_back();
                }
                handled = true;
                break;
            }
            mc = mc->getParent();
        }

        if (!handled)
        {
            for (int i = (int)_popupStack.size() - 1; i >= 0; i--)
            {
                GObject* popup = _popupStack[i].ptr<GObject>();
                if (popup)
                {
                    _justClosedPopups.push_back(WeakPtr(popup));
                    closePopup(popup);
                }
            }
            _popupStack.clear();
        }
    }
}

bool GRoot::hasAnyPopup()
{
    return !_popupStack.empty();
}

hkvVec2 GRoot::getPoupPosition(GObject * popup, GObject * target, PopupDirection dir)
{
    hkvVec2 pos;
    hkvVec2 size(0, 0);
    if (target != nullptr)
    {
        pos = target->localToGlobal(hkvVec2::ZeroVector());
        pos = this->globalToLocal(pos);
        size = target->localToGlobal(target->getSize());
        size = this->globalToLocal(size);
        size -= pos;
    }
    else
    {
        pos = globalToLocal(StageInst->getTouchPosition());
    }
    float xx, yy;
    xx = pos.x;
    if (xx + popup->getWidth() > getWidth())
        xx = xx + size.x - popup->getWidth();
    yy = pos.y + size.y;
    if ((dir == PopupDirection::AUTO && yy + popup->getHeight() > getHeight())
        || dir == PopupDirection::UP)
    {
        yy = pos.y - popup->getHeight() - 1;
        if (yy < 0)
        {
            yy = 0;
            xx += size.x / 2;
        }
    }

    return hkvVec2(hkvMath::round(xx), hkvMath::round(yy));
}

void GRoot::showTooltips(const std::string & msg)
{
    if (_defaultTooltipWin == nullptr)
    {
        const std::string& resourceURL = UIConfig::tooltipsWin;
        if (resourceURL.empty())
        {
            CCLOGWARN("FairyGUI: UIConfig.tooltipsWin not defined");
            return;
        }

        _defaultTooltipWin = UIPackage::createObjectFromURL(resourceURL);
        _defaultTooltipWin->setTouchable(false);
        _defaultTooltipWin->retain();
    }

    _defaultTooltipWin->setText(msg);
    showTooltipsWin(_defaultTooltipWin);
}

void GRoot::showTooltipsWin(GObject * tooltipWin)
{
    hideTooltips();

    _tooltipWin = tooltipWin;
    schedule(SCHEDULE_SELECTOR(GRoot::doShowTooltipsWin), 0, 0, 0.1f);
}

void GRoot::doShowTooltipsWin(float)
{
    if (_tooltipWin == nullptr)
        return;

    hkvVec2 pt = StageInst->getTouchPosition();
    float xx = pt.x + 10;
    float yy = pt.y + 20;

    pt = globalToLocal(hkvVec2(xx, yy));
    xx = pt.x;
    yy = pt.y;

    if (xx + _tooltipWin->getWidth() > getWidth())
        xx = xx - _tooltipWin->getWidth();
    if (yy + _tooltipWin->getHeight() > getHeight())
    {
        yy = yy - _tooltipWin->getHeight() - 1;
        if (yy < 0)
            yy = 0;
    }

    _tooltipWin->setPosition(hkvMath::round(xx), hkvMath::round(yy));
    addChild(_tooltipWin);
}

void GRoot::hideTooltips()
{
    if (_tooltipWin != nullptr)
    {
        if (_tooltipWin->getParent() != nullptr)
            removeChild(_tooltipWin);
        _tooltipWin = nullptr;
    }
}

void GRoot::onTouchBegin(EventContext* context)
{
    if (_tooltipWin != nullptr)
        hideTooltips();

    checkPopups();
}

void GRoot::onWindowSizeChanged(EventContext* context)
{
    const hkvVec2& screenSize = StageInst->getSize();
    setSize((int)hkvMath::ceil(screenSize.x / _scaleFactor), (int)hkvMath::ceil(screenSize.y / _scaleFactor));
    setScale(_scaleFactor, _scaleFactor);
}

NS_FGUI_END