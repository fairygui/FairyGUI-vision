#include "GComponent.h"
#include "UIObjectFactory.h"
#include "GGroup.h"
#include "UIPackage.h"
#include "GButton.h"
#include "utils/ToolSet.h"
#include "core/HitTest.h"

NS_FGUI_BEGIN

GComponent::GComponent() :
    _container(nullptr),
    _scrollPane(nullptr),
    _childrenRenderOrder(ChildrenRenderOrder::ASCENT),
    _apexIndex(0),
    _boundsChanged(false),
    _trackBounds(false),
    _sortingChildCount(0),
    _applyingController(nullptr),
    _buildingDisplayList(false),
    _alignOffset(0, 0)
{
}

GComponent::~GComponent()
{
    for (auto &child : _children)
        child->_parent = nullptr;

    _children.clear();
    _controllers.clear();
    _transitions.clear();

    CC_SAFE_RELEASE(_container);
    CC_SAFE_RELEASE(_scrollPane);
}

void GComponent::handleInit()
{
    _displayObject = DisplayObject::create();
    _displayObject->retain();

    _container = DisplayObject::create();
    _container->retain();

    _displayObject->addChild(_container);
}

GObject* GComponent::addChild(GObject* child)
{
    addChildAt(child, (int)_children.size());
    return child;
}

GObject* GComponent::addChildAt(GObject* child, int index)
{
    CCASSERT(child != nullptr, "Argument must be non-nil");

    if (child->_parent == this)
    {
        setChildIndex(child, index);
    }
    else
    {
        child->retain();
        child->removeFromParent();
        child->_parent = this;

        int cnt = (int)_children.size();
        if (child->_sortingOrder != 0)
        {
            _sortingChildCount++;
            index = getInsertPosForSortingChild(child);
        }
        else if (_sortingChildCount > 0)
        {
            if (index > (cnt - _sortingChildCount))
                index = cnt - _sortingChildCount;
        }

        if (index == cnt)
            _children.pushBack(child);
        else
            _children.insert(index, child);

        child->release();

        childStateChanged(child);
        setBoundsChangedFlag();
        if (child->_group != nullptr)
            child->_group->setBoundsChangedFlag(true);
    }
    return child;
}

int GComponent::getInsertPosForSortingChild(GObject* target)
{
    size_t cnt = _children.size();
    size_t i;
    for (i = 0; i < cnt; i++)
    {
        GObject* child = _children.at(i);
        if (child == target)
            continue;

        if (target->_sortingOrder < child->_sortingOrder)
            break;
    }
    return (int)i;
}

void GComponent::removeChild(GObject* child)
{
    CCASSERT(child != nullptr, "Argument must be non-nil");

    int childIndex = (int)_children.getIndex(child);
    if (childIndex != -1)
        removeChildAt(childIndex);
}

void GComponent::removeChildAt(int index)
{
    GObject* child = _children.at(index);

    child->_parent = nullptr;

    if (child->_sortingOrder != 0)
        _sortingChildCount--;

    child->setGroup(nullptr);
    if (child->_displayObject != nullptr && child->_displayObject->getParent() != nullptr)
    {
        _container->removeChild(child->_displayObject);
        if (_childrenRenderOrder == ChildrenRenderOrder::ARCH)
            scheduleOnce(SCHEDULE_SELECTOR(GComponent::buildNativeDisplayList));
    }

    _children.erase(index);
    setBoundsChangedFlag();
}

void GComponent::removeChildren(int beginIndex, int endIndex)
{
    if (endIndex < 0 || endIndex >= _children.size())
        endIndex = (int)_children.size() - 1;

    for (int i = beginIndex; i <= endIndex; ++i)
        removeChildAt(beginIndex);
}

GObject* GComponent::getChildAt(int index) const
{
    return _children.at(index);
}

GObject* GComponent::getChild(const std::string& name) const
{
    for (const auto& child : _children)
    {
        if (child->name.compare(name) == 0)
            return child;
    }

    return nullptr;
}

GObject* GComponent::getChildInGroup(const GGroup* group, const std::string& name) const
{
    CCASSERT(group != nullptr, "Argument must be non-nil");

    for (const auto& child : _children)
    {
        if (child->_group == group && child->name.compare(name) == 0)
            return child;
    }

    return nullptr;
}

GObject* GComponent::getChildById(const std::string& id) const
{
    for (const auto& child : _children)
    {
        if (child->id.compare(id) == 0)
            return child;
    }

    return nullptr;
}

int GComponent::getChildIndex(const GObject* child) const
{
    CCASSERT(child != nullptr, "Argument must be non-nil");

    return (int)_children.getIndex((GObject*)child);
}

void GComponent::setChildIndex(GObject* child, int index)
{
    CCASSERT(child != nullptr, "Argument must be non-nil");

    int oldIndex = (int)_children.getIndex(child);
    CCASSERT(oldIndex != -1, "Not a child of this container");

    if (child->_sortingOrder != 0) //no effect
        return;

    int cnt = (int)_children.size();
    if (_sortingChildCount > 0)
    {
        if (index > (cnt - _sortingChildCount - 1))
            index = cnt - _sortingChildCount - 1;
    }

    moveChild(child, oldIndex, index);
}

int GComponent::setChildIndexBefore(GObject* child, int index)
{
    CCASSERT(child != nullptr, "Argument must be non-nil");

    int oldIndex = (int)_children.getIndex(child);
    CCASSERT(oldIndex != -1, "Not a child of this container");

    if (child->_sortingOrder != 0) //no effect
        return oldIndex;

    int cnt = (int)_children.size();
    if (_sortingChildCount > 0)
    {
        if (index > (cnt - _sortingChildCount - 1))
            index = cnt - _sortingChildCount - 1;
    }

    if (oldIndex < index)
        return moveChild(child, oldIndex, index - 1);
    else
        return moveChild(child, oldIndex, index);
}

int GComponent::moveChild(GObject* child, int oldIndex, int index)
{
    int cnt = (int)_children.size();
    if (index > cnt)
        index = cnt;

    if (oldIndex == index)
        return oldIndex;

    child->retain();
    _children.erase(oldIndex);
    if (index >= cnt)
        _children.pushBack(child);
    else
        _children.insert(index, child);
    child->release();

    if (child->_displayObject != nullptr && child->_displayObject->getParent() != nullptr)
    {
        if (_childrenRenderOrder == ChildrenRenderOrder::ASCENT)
        {
            int displayIndex = 0;
            for (int i = 0; i < index; i++)
            {
                GObject* g = _children.at(i);
                if (g->_displayObject && g->_displayObject->getParent())
                    displayIndex++;
            }
            _container->setChildIndex(child->_displayObject, displayIndex);
        }
        else if (_childrenRenderOrder == ChildrenRenderOrder::DESCENT)
        {
            int displayIndex = 0;
            for (int i = cnt - 1; i > index; i--)
            {
                GObject* g = _children.at(i);
                if (g->_displayObject && g->_displayObject->getParent())
                    displayIndex++;
            }
            _container->setChildIndex(child->_displayObject, displayIndex);
        }
        else
            scheduleOnce(SCHEDULE_SELECTOR(GComponent::buildNativeDisplayList));

        setBoundsChangedFlag();
    }

    return index;
}

void GComponent::swapChildren(GObject* child1, GObject* child2)
{
    CCASSERT(child1 != nullptr, "Argument1 must be non-nil");
    CCASSERT(child2 != nullptr, "Argument2 must be non-nil");

    int index1 = (int)_children.getIndex(child1);
    int index2 = (int)_children.getIndex(child2);

    CCASSERT(index1 != -1, "Not a child of this container");
    CCASSERT(index2 != -1, "Not a child of this container");

    swapChildrenAt(index1, index2);
}

void GComponent::swapChildrenAt(int index1, int index2)
{
    GObject* child1 = _children.at(index1);
    GObject* child2 = _children.at(index2);

    setChildIndex(child1, index2);
    setChildIndex(child2, index1);
}

int GComponent::numChildren() const
{
    return (int)_children.size();
}

bool GComponent::isAncestorOf(const GObject * obj) const
{
    if (obj == nullptr)
        return false;

    GComponent* p = obj->_parent;
    while (p != nullptr)
    {
        if (p == this)
            return true;

        p = p->_parent;
    }
    return false;
}

bool GComponent::isChildInView(GObject * child)
{
    if (_scrollPane != nullptr)
    {
        return _scrollPane->isChildInView(child);
    }
    else if (_displayObject->getClipRect().GetSizeX() > 0)
    {
        return child->_position.x + child->_size.x >= 0 && child->_position.x <= _size.x
            && child->_position.y + child->_size.y >= 0 && child->_position.y <= _size.y;
    }
    else
        return true;
}

int GComponent::getFirstChildInView()
{
    int i = 0;
    for (auto &child : _children)
    {

        if (isChildInView(child))
            return i;
        i++;
    }
    return -1;
}

GController* GComponent::getController(const std::string& name) const
{
    for (const auto& c : _controllers)
    {
        if (c->getName().compare(name) == 0)
            return c;
    }

    return nullptr;
}

void GComponent::addController(GController* c)
{
    CCASSERT(c != nullptr, "Argument must be non-nil");

    _controllers.pushBack(c);
}

GController * GComponent::getControllerAt(int index) const
{
    return _controllers.at(index);
}

void GComponent::removeController(GController* c)
{
    CCASSERT(c != nullptr, "Argument must be non-nil");

    ssize_t index = _controllers.getIndex(c);
    CCASSERT(index != -1, "controller not exists");

    c->setParent(nullptr);
    applyController(c);
    _controllers.erase(index);
}

void GComponent::applyController(GController * c)
{
    _applyingController = c;

    for (const auto& child : _children)
        child->handleControllerChanged(c);

    _applyingController = nullptr;

    c->runActions();
}

void GComponent::applyAllControllers()
{
    for (const auto& c : _controllers)
        applyController(c);
}

Transition * GComponent::getTransition(const std::string & name) const
{
    for (const auto& c : _transitions)
    {
        if (c->name.compare(name) == 0)
            return c;
    }

    return nullptr;
}

Transition * GComponent::getTransitionAt(int index) const
{
    return _transitions.at(index);
}

void GComponent::adjustRadioGroupDepth(GObject* obj, GController* c)
{
    ssize_t cnt = (ssize_t)_children.size();
    ssize_t i;
    GObject* child;
    ssize_t myIndex = -1, maxIndex = -1;
    for (i = 0; i < cnt; i++)
    {
        child = _children.at(i);
        if (child == obj)
        {
            myIndex = i;
        }
        else if (dynamic_cast<GButton*>(child)
            && ((GButton *)child)->getRelatedController() == c)
        {
            if (i > maxIndex)
                maxIndex = i;
        }
    }
    if (myIndex < maxIndex)
    {
        if (_applyingController != nullptr)
            _children.at(maxIndex)->handleControllerChanged(_applyingController);
        swapChildrenAt((int)myIndex, (int)maxIndex);
    }
}


void GComponent::setOpaque(bool value)
{
    _displayObject->setOpaque(value);
}

void GComponent::setMargin(const Margin & value)
{
    _margin = value;
    if (_displayObject->getClipRect().GetSizeX() > 0 && _scrollPane == nullptr) //如果scrollPane不为空，则HandleSizeChanged里面的处理会促使ScrollPane处理
        _container->setPosition(_margin.left + _alignOffset.x, _margin.top + _alignOffset.y);
    handleSizeChanged();
}

void GComponent::setChildrenRenderOrder(ChildrenRenderOrder value)
{
    if (_childrenRenderOrder != value)
    {
        _childrenRenderOrder = value;
        scheduleOnce(SCHEDULE_SELECTOR(GComponent::buildNativeDisplayList));
    }
}

void GComponent::setApexIndex(int value)
{
    if (_apexIndex != value)
    {
        _apexIndex = value;

        if (_childrenRenderOrder == ChildrenRenderOrder::ARCH)
            scheduleOnce(SCHEDULE_SELECTOR(GComponent::buildNativeDisplayList));
    }
}

DisplayObject* GComponent::getMask() const
{
    return nullptr;
}

void GComponent::setMask(DisplayObject * value, bool inverted)
{

}

float GComponent::getViewWidth() const
{
    if (_scrollPane != nullptr)
        return _scrollPane->getViewSize().x;
    else
        return _size.x - _margin.left - _margin.right;
}

void GComponent::setViewWidth(float value)
{
    if (_scrollPane != nullptr)
        _scrollPane->setViewWidth(value);
    else
        setWidth(value + _margin.left + _margin.right);
}

float GComponent::getViewHeight() const
{
    if (_scrollPane != nullptr)
        return _scrollPane->getViewSize().y;
    else
        return _size.y - _margin.top - _margin.bottom;
}

void GComponent::setViewHeight(float value)
{
    if (_scrollPane != nullptr)
        _scrollPane->setViewHeight(value);
    else
        setHeight(value + _margin.top + _margin.bottom);
}

void GComponent::setBoundsChangedFlag()
{
    if (_scrollPane == nullptr && !_trackBounds)
        return;

    _boundsChanged = true;
    scheduleOnce(SCHEDULE_SELECTOR(GComponent::doUpdateBounds));
}

void GComponent::ensureBoundsCorrect()
{
    if (_boundsChanged)
        updateBounds();
}

void GComponent::updateBounds()
{
    float ax, ay, aw, ah;
    if (_children.size() > 0)
    {
        ax = FLT_MAX;
        ay = FLT_MAX;
        float ar = -FLT_MAX, ab = -FLT_MAX;
        float tmp;

        int cnt = (int)_children.size();
        for (int i = 0; i < cnt; ++i)
        {
            GObject* child = _children.at(i);
            tmp = child->getX();
            if (tmp < ax)
                ax = tmp;
            tmp = child->getY();
            if (tmp < ay)
                ay = tmp;
            tmp = child->getX() + child->getWidth();
            if (tmp > ar)
                ar = tmp;
            tmp = child->getY() + child->getHeight();
            if (tmp > ab)
                ab = tmp;
        }
        aw = ar - ax;
        ah = ab - ay;
    }
    else
    {
        ax = 0;
        ay = 0;
        aw = 0;
        ah = 0;
    }
    setBounds(ax, ay, aw, ah);
}

void GComponent::setBounds(float ax, float ay, float aw, float ah)
{
    _boundsChanged = false;
    if (_scrollPane != nullptr)
        _scrollPane->setContentSize(ceil(ax + aw), ceil(ay + ah));
}

void GComponent::doUpdateBounds(float)
{
    if (_boundsChanged)
        updateBounds();
}

void GComponent::childStateChanged(GObject* child)
{
    if (_buildingDisplayList)
        return;

    int cnt = (int)_children.size();

    if (dynamic_cast<GGroup*>(child) != nullptr)
    {
        for (int i = 0; i < cnt; ++i)
        {
            GObject* g = _children.at(i);
            if (g->_group == child)
                childStateChanged(g);
        }
    }

    if (child->_displayObject == nullptr)
        return;

    if (child->internalVisible())
    {
        if (child->_displayObject->getParent() == nullptr)
        {
            if (_childrenRenderOrder == ChildrenRenderOrder::ASCENT)
            {
                int index = 0;
                for (int i = 0; i < cnt; i++)
                {
                    GObject* g = _children.at(i);
                    if (g == child)
                        break;

                    if (g->_displayObject && g->_displayObject->getParent())
                        index++;
                }
                _container->addChildAt(child->_displayObject, index);
            }
            else if (_childrenRenderOrder == ChildrenRenderOrder::DESCENT)
            {
                int index = 0;
                for (int i = cnt - 1; i >= 0; i--)
                {
                    GObject* g = _children.at(i);
                    if (g == child)
                        break;

                    if (g->_displayObject && g->_displayObject->getParent())
                        index++;
                }
                _container->addChildAt(child->_displayObject, index);
            }
            else
            {
                scheduleOnce(SCHEDULE_SELECTOR(GComponent::buildNativeDisplayList));
            }
        }
    }
    else
    {
        if (child->_displayObject->getParent() != nullptr)
        {
            _container->removeChild(child->_displayObject);
            if (_childrenRenderOrder == ChildrenRenderOrder::ARCH)
            {
                scheduleOnce(SCHEDULE_SELECTOR(GComponent::buildNativeDisplayList));
            }
        }
    }
}

void GComponent::childSortingOrderChanged(GObject* child, int oldValue, int newValue)
{
    if (newValue == 0)
    {
        _sortingChildCount--;
        setChildIndex(child, (int)_children.size());
    }
    else
    {
        if (oldValue == 0)
            _sortingChildCount++;

        int oldIndex = (int)_children.getIndex(child);
        int index = getInsertPosForSortingChild(child);
        if (oldIndex < index)
            moveChild(child, oldIndex, index - 1);
        else
            moveChild(child, oldIndex, index);
    }
}

void GComponent::buildNativeDisplayList(float)
{
    int cnt = (int)_children.size();
    if (cnt == 0)
        return;

    switch (_childrenRenderOrder)
    {
    case ChildrenRenderOrder::ASCENT:
    {
        for (int i = 0; i < cnt; i++)
        {
            GObject* child = _children.at(i);
            if (child->_displayObject != nullptr && child->internalVisible())
                _container->addChild(child->_displayObject);
        }
    }
    break;
    case ChildrenRenderOrder::DESCENT:
    {
        for (int i = cnt - 1; i >= 0; i--)
        {
            GObject* child = _children.at(i);
            if (child->_displayObject != nullptr && child->internalVisible())
                _container->addChild(child->_displayObject);
        }
    }
    break;

    case ChildrenRenderOrder::ARCH:
    {
        for (int i = 0; i < _apexIndex; i++)
        {
            GObject* child = _children.at(i);
            if (child->_displayObject != nullptr && child->internalVisible())
                _container->addChild(child->_displayObject);
        }
        for (int i = cnt - 1; i >= _apexIndex; i--)
        {
            GObject* child = _children.at(i);
            if (child->_displayObject != nullptr && child->internalVisible())
                _container->addChild(child->_displayObject);
        }
    }
    break;
    }
}

void GComponent::scrollPaneUpdate(float dt)
{
    _scrollPane->tweenUpdate(dt);
}

void GComponent::refreshScrollPane(float)
{
    _scrollPane->refresh();
}

void GComponent::onShowScrollBar(float)
{
    _scrollPane->onShowScrollBar();
}

hkvVec2 GComponent::getSnappingPosition(const hkvVec2 & pt)
{
    int cnt = (int)_children.size();
    if (cnt == 0)
        return pt;

    ensureBoundsCorrect();

    GObject* obj = nullptr;

    hkvVec2 ret = pt;

    int i = 0;
    if (ret.y != 0)
    {
        for (; i < cnt; i++)
        {
            obj = _children.at(i);
            if (ret.y < obj->getY())
            {
                if (i == 0)
                {
                    ret.y = 0;
                    break;
                }
                else
                {
                    GObject* prev = _children.at(i - 1);
                    if (ret.y < prev->getY() + prev->getHeight() / 2) //top half part
                        ret.y = prev->getY();
                    else //bottom half part
                        ret.y = obj->getY();
                    break;
                }
            }
        }

        if (i == cnt)
            ret.y = obj->getY();
    }

    if (ret.x != 0)
    {
        if (i > 0)
            i--;
        for (; i < cnt; i++)
        {
            obj = _children.at(i);
            if (ret.x < obj->getX())
            {
                if (i == 0)
                {
                    ret.x = 0;
                    break;
                }
                else
                {
                    GObject* prev = _children.at(i - 1);
                    if (ret.x < prev->getX() + prev->getWidth() / 2) // top half part
                        ret.x = prev->getX();
                    else//bottom half part
                        ret.x = obj->getX();
                    break;
                }
            }
        }
        if (i == cnt)
            ret.x = obj->getX();
    }

    return ret;
}

void GComponent::setupOverflow(OverflowType overflow)
{
    if (overflow == OverflowType::HIDDEN)
    {
        _displayObject->setClipRect(VRectanglef(_margin.left, _margin.top,
            _size.x - _margin.right, _size.y - _margin.bottom));
    }

    _container->setPosition(_margin.left, _margin.top);
}

void GComponent::setupScroll(const Margin& scrollBarMargin,
    ScrollType scroll, ScrollBarDisplayType scrollBarDisplay, int flags,
    const std::string& vtScrollBarRes, const std::string& hzScrollBarRes,
    const std::string& headerRes, const std::string& footerRes)
{
    _scrollPane = new ScrollPane(this, scroll, scrollBarMargin, scrollBarDisplay, flags, vtScrollBarRes, hzScrollBarRes, headerRes, footerRes);
}

void GComponent::handleSizeChanged()
{
    GObject::handleSizeChanged();

    if (_scrollPane != nullptr)
        _scrollPane->onOwnerSizeChanged();
    else
        _container->setPosition(_margin.left, _margin.top);

    if (_displayObject->getClipRect().GetSizeX() > 0)
        _displayObject->setClipRect(VRectanglef(_margin.left, _margin.top,
            _size.x - _margin.right, _size.y - _margin.bottom));

    IHitTest* hitArea = _displayObject->getHitArea();
    if (hitArea)
    {
        PixelHitTest* test = dynamic_cast<PixelHitTest*>(hitArea);
        if (sourceSize.x != 0)
            test->scaleX = _size.x / sourceSize.x;
        if (sourceSize.y != 0)
            test->scaleY = _size.y / sourceSize.y;
    }
}

void GComponent::handleGrayedChanged()
{
    GObject::handleGrayedChanged();

    GController* cc = getController("grayed");
    if (cc != nullptr)
        cc->setSelectedIndex(isGrayed() ? 1 : 0);
    else
    {
        for (auto &child : _children)
            child->handleGrayedChanged();
    }
}

void GComponent::handleControllerChanged(GController* c)
{
    GObject::handleControllerChanged(c);

    if (_scrollPane != nullptr)
        _scrollPane->handleControllerChanged(c);
}

void GComponent::onAddedToStage(EventContext* context)
{
    if (!_transitions.empty())
    {
        for (auto &trans : _transitions)
        {
            if (trans->isAutoPlay())
                trans->play(trans->autoPlayRepeat, trans->autoPlayDelay, nullptr);
        }
    }
}

void GComponent::onRemoveFromStage(EventContext* context)
{
    if (!_transitions.empty())
    {
        for (auto &trans : _transitions)
            trans->OnOwnerRemovedFromStage();
    }
}

void GComponent::constructFromResource()
{
    constructFromResource(nullptr, 0);
}

void GComponent::constructFromResource(std::vector<GObject*>* objectPool, int poolIndex)
{
    TXMLElement* xml = _packageItem->componentData->RootElement();

    _underConstruct = true;

    hkvVec2 v2(0, 0);
    hkvVec4 v4(0, 0, 0, 0);
    const char *p;

    p = xml->Attribute("size");
    ToolSet::splitString(p, ',', v2, true);
    initSize = sourceSize = v2;

    setSize(sourceSize.x, sourceSize.y);

    p = xml->Attribute("restrictSize");
    if (p)
    {
        ToolSet::splitString(p, ',', v4, true);
        minSize.x = v4.x;
        minSize.y = v4.z;
        maxSize.x = v4.y;
        maxSize.y = v4.w;
    }

    p = xml->Attribute("pivot");
    if (p)
    {
        ToolSet::splitString(p, ',', v2);
        setPivot(v2.x, v2.y, xml->BoolAttribute("anchor"));
    }

    p = xml->Attribute("opaque");
    if (p)
        setOpaque(strcmp(p, "true") == 0);
    else
        setOpaque(true);

    p = xml->Attribute("hitTest");
    if (p)
    {
        std::vector<std::string> arr;
        ToolSet::splitString(p, ',', arr);
        PixelHitTestData* hitTestData = _packageItem->owner->getPixelHitTestData(arr[0]);
        if (hitTestData != nullptr)
            setHitArea(new PixelHitTest(hitTestData, atoi(arr[1].c_str()), atoi(arr[2].c_str())));
    }

    OverflowType overflow;
    p = xml->Attribute("overflow");
    if (p)
        overflow = ToolSet::parseOverflowType(p);
    else
        overflow = OverflowType::VISIBLE;

    p = xml->Attribute("margin");
    if (p)
    {
        ToolSet::splitString(p, ',', v4);
        _margin.setMargin(v4.z, v4.x, v4.w, v4.y);
    }

    if (overflow == OverflowType::SCROLL)
    {
        ScrollType scroll;
        p = xml->Attribute("scroll");
        if (p)
            scroll = ToolSet::parseScrollType(p);
        else
            scroll = ScrollType::VERTICAL;

        ScrollBarDisplayType scrollBarDisplay;
        p = xml->Attribute("scrollBar");
        if (p)
            scrollBarDisplay = ToolSet::parseScrollBarDisplayType(p);
        else
            scrollBarDisplay = ScrollBarDisplayType::DEFAULT;

        int scrollBarFlags = xml->IntAttribute("scrollBarFlags");

        Margin scrollBarMargin;
        p = xml->Attribute("scrollBarMargin");
        if (p)
        {
            ToolSet::splitString(p, ',', v4);
            scrollBarMargin.setMargin(v4.z, v4.x, v4.w, v4.y);
        }

        std::string  vtScrollBarRes;
        std::string hzScrollBarRes;
        p = xml->Attribute("scrollBarRes");
        if (p)
            ToolSet::splitString(p, ',', vtScrollBarRes, hzScrollBarRes);

        std::string headerRes;
        std::string footerRes;
        p = xml->Attribute("ptrRes");
        if (p)
            ToolSet::splitString(p, ',', headerRes, footerRes);

        setupScroll(scrollBarMargin, scroll, scrollBarDisplay, scrollBarFlags,
            vtScrollBarRes, hzScrollBarRes, headerRes, footerRes);
    }
    else
        setupOverflow(overflow);

    _buildingDisplayList = true;

    TXMLElement* exml = xml->FirstChildElement("controller");
    while (exml)
    {
        GController* controller = new GController();
        _controllers.pushBack(controller);
        controller->release();
        controller->setParent(this);
        controller->setup(exml);

        exml = exml->NextSiblingElement("controller");
    }

    GObject* child;
    std::vector<DisplayListItem*>& displayList = *_packageItem->displayList;
    size_t childCount = displayList.size();
    for (size_t i = 0; i < childCount; i++)
    {
        DisplayListItem& di = *displayList[i];
        if (objectPool)
        {
            child = (*objectPool)[poolIndex + i];
            _children.pushBack(child);
        }
        else if (di.packageItem)
        {
            di.packageItem->load();
            child = UIObjectFactory::newObject(di.packageItem);
            _children.pushBack(child);
            child->constructFromResource();
        }
        else
        {
            child = UIObjectFactory::newObject(di.type);
            _children.pushBack(child);
        }

        child->_underConstruct = true;
        child->setup_BeforeAdd(di.desc);
        child->_parent = this;
    }

    _relations->setup(xml);

    for (size_t i = 0; i < childCount; i++)
        _children.at(i)->_relations->setup(displayList[i]->desc);

    for (size_t i = 0; i < childCount; i++)
    {
        child = _children.at(i);
        child->setup_AfterAdd(displayList[i]->desc);
        child->_underConstruct = false;
    }

    p = xml->Attribute("mask");
    if (p)
    {
        bool inverted = xml->BoolAttribute("reversedMask");
        setMask(getChildById(p)->displayObject(), inverted);
    }

    exml = xml->FirstChildElement("transition");
    while (exml)
    {
        Transition* trans = new Transition(this, (int)_transitions.size());
        _transitions.pushBack(trans);
        trans->release();
        trans->setup(exml);

        exml = exml->NextSiblingElement("transition");
    }
    if (!_transitions.empty())
    {
        _container->addListener(UIEventType::AddedToStage, CALLBACK_1(GComponent::onAddedToStage, this));
        _container->addListener(UIEventType::RemoveFromStage, CALLBACK_1(GComponent::onRemoveFromStage, this));
    }

    applyAllControllers();

    _buildingDisplayList = false;
    _underConstruct = false;

    buildNativeDisplayList(0);
    setBoundsChangedFlag();

    constructFromXML(xml);
}

void GComponent::constructFromXML(TXMLElement * xml)
{
}

void GComponent::setup_AfterAdd(TXMLElement * xml)
{
    GObject::setup_AfterAdd(xml);

    const char *p;

    if (_scrollPane != nullptr && _scrollPane->isPageMode())
    {
        p = xml->Attribute("pageController");
        if (p)
            _scrollPane->setPageController(_parent->getController(p));
    }

    p = xml->Attribute("controller");
    if (p)
    {
        std::vector<std::string> arr;
        ToolSet::splitString(p, ',', arr);
        size_t cnt = arr.size();
        for (size_t i = 0; i < cnt; i += 2)
        {
            GController* cc = getController(arr[i]);
            if (cc != nullptr)
                cc->setSelectedPageId(arr[i + 1]);
        }
    }
}

NS_FGUI_END

