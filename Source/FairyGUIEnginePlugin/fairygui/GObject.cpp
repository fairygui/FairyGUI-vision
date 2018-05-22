#include "GObject.h"
#include "GGroup.h"
#include "GList.h"
#include "GRoot.h"
#include "UIPackage.h"
#include "UIConfig.h"
#include "gears/GearXY.h"
#include "gears/GearSize.h"
#include "gears/GearColor.h"
#include "gears/GearAnimation.h"
#include "gears/GearLook.h"
#include "gears/GearText.h"
#include "gears/GearIcon.h"
#include "gears/GearDisplay.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

GObject* GObject::_draggingObject = nullptr;

static hkvVec2 sGlobalDragStart;
static VRectanglef sGlobalRect;
static bool sUpdateInDragging;

GObject::GObject() :
    _position(0, 0),
    _scale(1, 1),
    _size(0, 0),
    sourceSize(0, 0),
    _rawSize(0, 0),
    minSize(0, 0),
    maxSize(0, 0),
    initSize(0, 0),
    _sizePercentInGroup(0.0f),
    _pivot(0, 0),
    _pivotAsAnchor(false),
    _alpha(1.0f),
    _rotation(0.0f),
    _visible(true),
    _internalVisible(true),
    _handlingController(false),
    _touchable(true),
    _grayed(false),
    _draggable(false),
    _dragBounds(nullptr),
    _sortingOrder(0),
    _focusable(false),
    _pixelSnapping(false),
    _group(nullptr),
    _parent(nullptr),
    _displayObject(nullptr),
    _sizeImplType(0),
    _underConstruct(false),
    _gearLocked(false),
    _packageItem(nullptr),
    _data(nullptr)
{
    std::stringstream ss;
    ss << _intID;
    id = ss.str();
    _relations = new Relations(this);

    for (int i = 0; i < 8; i++)
        _gears[i] = nullptr;
}

GObject::~GObject()
{
    removeFromParent();

    if (_displayObject)
        _displayObject->setSpectator(nullptr);

    CC_SAFE_RELEASE(_displayObject);
    for (int i = 0; i < 8; i++)
        CC_SAFE_DELETE(_gears[i]);
    CC_SAFE_DELETE(_relations);
    CC_SAFE_DELETE(_dragBounds);
}

bool GObject::init()
{
    handleInit();
    if (_displayObject)
        _displayObject->setSpectator(this);
    return true;
}

void GObject::setX(float value)
{
    setPosition(value, _position.y);
}

void GObject::setY(float value)
{
    setPosition(_position.x, value);
}

void GObject::setPosition(float xv, float yv)
{
    if (_position.x != xv || _position.y != yv)
    {
        float dx = xv - _position.x;
        float dy = yv - _position.y;
        _position.x = xv;
        _position.y = yv;

        handlePositionChanged();

        GGroup* g = dynamic_cast<GGroup*>(this);
        if (g != nullptr)
            g->moveChildren(dx, dy);

        updateGear(1);

        if (_parent != nullptr && dynamic_cast<GList*>(_parent) == nullptr)
        {
            _parent->setBoundsChangedFlag();
            if (_group != nullptr)
                _group->setBoundsChangedFlag();

            dispatchEvent(UIEventType::PositionChange);
        }

        if (_draggingObject == this && !sUpdateInDragging)
            sGlobalRect = localToGlobal(VRectanglef(0, 0, _size.x, _size.y));
    }
}


float GObject::getXMin() const
{
    return _pivotAsAnchor ? (_position.x - _size.x * _pivot.x) : _position.x;
}

void GObject::setXMin(float value)
{
    if (_pivotAsAnchor)
        setPosition(value + _size.x * _pivot.x, _position.y);
    else
        setPosition(value, _position.y);
}

float GObject::getYMin() const
{
    return _pivotAsAnchor ? (_position.y - _size.y * _pivot.y) : _position.y;
}

void GObject::setYMin(float value)
{
    if (_pivotAsAnchor)
        setPosition(_position.x, value + _size.y * _pivot.y);
    else
        setPosition(_position.x, value);
}

void GObject::setPixelSnapping(bool value)
{
    if (_pixelSnapping != value)
    {
        _pixelSnapping = value;
        handlePositionChanged();
    }
}

void GObject::setSize(float wv, float hv, bool ignorePivot /*= false*/)
{
    if (_rawSize.x != wv || _rawSize.y != hv)
    {
        _rawSize.x = wv;
        _rawSize.y = hv;
        if (wv < minSize.x)
            wv = minSize.x;
        else if (maxSize.x > 0 && wv > maxSize.x)
            wv = maxSize.x;
        if (hv < minSize.y)
            hv = minSize.y;
        else if (maxSize.y > 0 && hv > maxSize.y)
            hv = maxSize.y;
        float dWidth = wv - _size.x;
        float dHeight = hv - _size.y;
        _size.x = wv;
        _size.y = hv;

        handleSizeChanged();

        if (_pivot.x != 0 || _pivot.y != 0)
        {
            if (!_pivotAsAnchor)
            {
                if (!ignorePivot)
                    setPosition(_position.x - _pivot.x * dWidth, _position.y - _pivot.y * dHeight);
                else
                    handlePositionChanged();
            }
            else
                handlePositionChanged();
        }
        else
            handlePositionChanged();

        GGroup* g = dynamic_cast<GGroup*>(this);
        if (g != nullptr)
            g->resizeChildren(dWidth, dHeight);

        updateGear(2);

        if (_parent != nullptr)
        {
            _relations->onOwnerSizeChanged(dWidth, dHeight, _pivotAsAnchor || !ignorePivot);
            _parent->setBoundsChangedFlag();
            if (_group != nullptr)
                _group->setBoundsChangedFlag(true);
        }

        dispatchEvent(UIEventType::SizeChange);
    }
}

void GObject::setSizeDirectly(float wv, float hv)
{
    _rawSize.x = wv;
    _rawSize.y = hv;
    if (wv < 0)
        wv = 0;
    if (hv < 0)
        hv = 0;
    _size.x = wv;
    _size.y = hv;
}

void GObject::center(bool restraint /*= false*/)
{
    GComponent* r;
    if (_parent != nullptr)
        r = _parent;
    else
        r = UIRoot;

    setPosition((int)((r->_size.x - _size.x) / 2), (int)((r->_size.y - _size.y) / 2));
    if (restraint)
    {
        addRelation(r, RelationType::Center_Center);
        addRelation(r, RelationType::Middle_Middle);
    }
}

void GObject::makeFullScreen()
{
    setSize(UIRoot->getWidth(), UIRoot->getHeight());
}

void GObject::setPivot(float xv, float yv, bool asAnchor)
{
    if (_pivot.x != xv || _pivot.y != yv || _pivotAsAnchor != asAnchor)
    {
        _pivot.set(xv, yv);
        _pivotAsAnchor = asAnchor;
        if (_displayObject != nullptr)
            _displayObject->setPivot(_pivot.x, _pivot.y);
        handlePositionChanged();
    }
}

void GObject::setScale(float xv, float yv)
{
    if (_scale.x != xv || _scale.y != yv)
    {
        _scale.x = xv;
        _scale.y = yv;
        handleScaleChanged();

        updateGear(2);
    }
}

void GObject::setSkewX(float value)
{
    if (_displayObject != nullptr)
        _displayObject->setSkewX(value);
}

void GObject::setSkewY(float value)
{
    if (_displayObject != nullptr)
        _displayObject->setSkewY(value);
}

void GObject::setRotation(float value)
{
    if (_rotation != value)
    {
        _rotation = value;
        if (_displayObject != nullptr)
            _displayObject->setRotation(_rotation);
        updateGear(3);
    }
}

void GObject::setAlpha(float value)
{
    if (_alpha != value)
    {
        _alpha = value;
        handleAlphaChanged();
        updateGear(3);
    }
}

void GObject::setGrayed(bool value)
{
    if (_grayed != value)
    {
        _grayed = value;
        handleGrayedChanged();
        updateGear(3);
    }
}

void GObject::setVisible(bool value)
{
    if (_visible != value)
    {
        _visible = value;
        handleVisibleChanged();
        if (_parent != nullptr)
            _parent->setBoundsChangedFlag();
    }
}

bool GObject::internalVisible() const
{
    return _internalVisible && (_group == nullptr || _group->internalVisible());
}

bool GObject::internalVisible2() const
{
    return _visible && (_group == nullptr || _group->internalVisible2());
}

void GObject::setTouchable(bool value)
{
    if (_touchable != value)
    {
        _touchable = value;
        updateGear(3);

        if (_displayObject != nullptr)
            _displayObject->setTouchable(_touchable);
    }
}

void GObject::setSortingOrder(int value)
{
    if (value < 0)
        value = 0;
    if (_sortingOrder != value)
    {
        int old = _sortingOrder;
        _sortingOrder = value;
        if (_parent != nullptr)
            _parent->childSortingOrderChanged(this, old, _sortingOrder);
    }
}

void GObject::setGroup(GGroup * value)
{
    if (_group != value)
    {
        if (_group != nullptr)
            _group->setBoundsChangedFlag(true);
        _group = value;
        if (_group != nullptr)
            _group->setBoundsChangedFlag(true);
        handleVisibleChanged();
        if (_parent)
            _parent->childStateChanged(this);
    }
}

const std::string& GObject::getText() const
{
    return STD_STRING_EMPTY;
}

void GObject::setText(const std::string & text)
{
}

const std::string & GObject::getIcon() const
{
    return STD_STRING_EMPTY;
}

void GObject::setIcon(const std::string & text)
{
}

void GObject::setTooltips(const std::string & value)
{
    _tooltips = value;
    if (!_tooltips.empty())
    {
        addListener(UIEventType::RollOver, CALLBACK_1(GObject::onRollOver, this), EventTag(this));
        addListener(UIEventType::RollOut, CALLBACK_1(GObject::onRollOut, this), EventTag(this));
    }
}

void GObject::onRollOver(EventContext* context)
{
    getRoot()->showTooltips(_tooltips);
}

void GObject::onRollOut(EventContext* context)
{
    getRoot()->hideTooltips();
}

void GObject::setDraggable(bool value)
{
    if (_draggable != value)
    {
        _draggable = value;
        initDrag();
    }
}

void GObject::setDragBounds(const VRectanglef & value)
{
    if (_dragBounds == nullptr)
        _dragBounds = new VRectanglef();
    *_dragBounds = value;
}

void GObject::startDrag(int touchId)
{
    dragBegin(touchId);
}

void GObject::stopDrag()
{
    dragEnd();
}

std::string GObject::getResourceURL() const
{
    if (_packageItem != nullptr)
        return "ui://" + _packageItem->owner->getId() + _packageItem->id;
    else
        return STD_STRING_EMPTY;
}

hkvVec2 GObject::localToGlobal(const hkvVec2 & pt)
{
    hkvVec2 pt2 = pt;
    if (_pivotAsAnchor)
    {
        pt2.x += _size.x*_pivot.x;
        pt2.y += _size.y*_pivot.y;
    }
    pt2 = _displayObject->localToGlobal(pt2);
    return pt2;
}

VRectanglef GObject::localToGlobal(const VRectanglef & rect)
{
    VRectanglef ret;
    ret.m_vMin = localToGlobal(rect.m_vMin);
    ret.m_vMax = localToGlobal(rect.m_vMax);
    return ret;
}

hkvVec2 GObject::globalToLocal(const hkvVec2 & pt)
{
    hkvVec2 pt2 = pt;
    pt2 = _displayObject->globalToLocal(pt2);
    if (_pivotAsAnchor)
    {
        pt2.x -= _size.x*_pivot.x;
        pt2.y -= _size.y*_pivot.y;
    }
    return pt2;
}

VRectanglef GObject::globalToLocal(const VRectanglef & rect)
{
    VRectanglef ret;
    ret.m_vMin = globalToLocal(rect.m_vMin);
    ret.m_vMax = globalToLocal(rect.m_vMax);
    return ret;
}

VRectanglef GObject::transformRect(const VRectanglef& rect, GObject * targetSpace)
{
    VRectanglef rect2 = rect;
    if (_pivotAsAnchor)
    {
        rect2.m_vMin.x += _size.x * _pivot.x;
        rect2.m_vMin.y += _size.y * _pivot.y;
    }
    return _displayObject->transformRect(rect2, targetSpace->displayObject());
}

void GObject::addRelation(GObject * target, RelationType relationType, bool usePercent)
{
    _relations->add(target, relationType, usePercent);
}

void GObject::removeRelation(GObject * target, RelationType relationType)
{
    _relations->remove(target, relationType);
}

GearBase* GObject::getGear(int index)
{
    GearBase* gear = _gears[index];
    if (gear == nullptr)
    {
        switch (index)
        {
        case 0:
            gear = new GearDisplay(this);
            break;
        case 1:
            gear = new GearXY(this);
            break;
        case 2:
            gear = new GearSize(this);
            break;
        case 3:
            gear = new GearLook(this);
            break;
        case 4:
            gear = new GearColor(this);
            break;
        case 5:
            gear = new GearAnimation(this);
            break;
        case 6:
            gear = new GearText(this);
            break;
        case 7:
            gear = new GearIcon(this);
            break;
        }
        _gears[index] = gear;
    }
    return gear;
}

void GObject::updateGear(int index)
{
    if (_underConstruct || _gearLocked)
        return;

    GearBase* gear = _gears[index];
    if (gear != nullptr && gear->getController() != nullptr)
        gear->updateState();
}

bool GObject::checkGearController(int index, GController* c)
{
    return _gears[index] != nullptr && _gears[index]->getController() == c;
}

void GObject::updateGearFromRelations(int index, float dx, float dy)
{
    if (_gears[index] != nullptr)
        _gears[index]->updateFromRelations(dx, dy);
}

uint32_t GObject::addDisplayLock()
{
    GearDisplay* gearDisplay = (GearDisplay*)_gears[0];
    if (gearDisplay != nullptr && gearDisplay->getController() != nullptr)
    {
        uint32_t ret = gearDisplay->addLock();
        checkGearDisplay();

        return ret;
    }
    else
        return 0;
}

void GObject::releaseDisplayLock(uint32_t token)
{
    GearDisplay* gearDisplay = (GearDisplay*)_gears[0];
    if (gearDisplay != nullptr && gearDisplay->getController() != nullptr)
    {
        gearDisplay->releaseLock(token);
        checkGearDisplay();
    }
}

void GObject::checkGearDisplay()
{
    if (_handlingController)
        return;

    bool connected = _gears[0] == nullptr || ((GearDisplay*)_gears[0])->isConnected();
    if (connected != _internalVisible)
    {
        _internalVisible = connected;
        if (_parent != nullptr)
            _parent->childStateChanged(this);
    }
}

bool GObject::onStage() const
{
    return _displayObject != nullptr && _displayObject->onStage();
}

GRoot* GObject::getRoot() const
{
    GObject* p = (GObject*)this;
    while (p->_parent != nullptr)
        p = p->_parent;

    GRoot* root = dynamic_cast<GRoot*>(p);
    if (root != nullptr)
        return root;
    else
        return UIRoot;
}

void GObject::removeFromParent()
{
    if (_parent != nullptr)
        _parent->removeChild(this);
}

void GObject::constructFromResource()
{
}

void GObject::handleInit()
{
}

void GObject::handlePositionChanged()
{
    if (_displayObject)
    {
        hkvVec2 pt = _position;
        if (!_pivotAsAnchor)
        {
            pt.x += _size.x * _pivot.x;
            pt.y += _size.y * _pivot.y;
        }
        if (_pixelSnapping)
        {
            pt.x = (int)pt.x;
            pt.y = (int)pt.y;
        }
        _displayObject->setLocation(pt.x, pt.y);
    }
}

void GObject::handleSizeChanged()
{
    if (_displayObject)
    {
        if (_sizeImplType == 0 || sourceSize.x == 0 || sourceSize.y == 0)
            _displayObject->setSize(_size.x, _size.y);
        else
            _displayObject->setScale(_scale.x * _size.x / sourceSize.x, _scale.y * _size.y / sourceSize.y);
    }
}

void GObject::handleScaleChanged()
{
    if (_displayObject != nullptr)
    {
        if (_sizeImplType == 0 || sourceSize.x == 0 || sourceSize.y == 0)
            _displayObject->setScale(_scale.x, _scale.y);
        else
            _displayObject->setScale(_scale.x * _size.x / sourceSize.x, _scale.y * _size.y / sourceSize.y);
    }
}

void GObject::handleAlphaChanged()
{
    if (_displayObject != nullptr)
        _displayObject->setAlpha(_alpha);
}

void GObject::handleGrayedChanged()
{
    _displayObject->setGrayed(_grayed);
}

void GObject::handleVisibleChanged()
{
    if (_displayObject != nullptr)
        _displayObject->setVisible(internalVisible2());
}

void GObject::handleControllerChanged(GController * c)
{
    _handlingController = true;
    for (int i = 0; i < 8; i++)
    {
        GearBase* gear = _gears[i];
        if (gear != nullptr && gear->getController() == c)
            gear->apply();
    }
    _handlingController = false;

    checkGearDisplay();
}

void GObject::setup_BeforeAdd(TXMLElement * xml)
{
    const char *p;
    hkvVec2 v2(0, 0);
    hkvVec4 v4(0, 0, 0, 0);

    p = xml->Attribute("id");
    if (p)
        id = p;

    p = xml->Attribute("name");
    if (p)
        name = p;

    p = xml->Attribute("xy");
    if (p)
    {
        ToolSet::splitString(p, ',', v2, true);
        setPosition(v2.x, v2.y);
    }

    p = xml->Attribute("size");
    if (p)
    {
        ToolSet::splitString(p, ',', v2, true);
        initSize = v2;
        setSize(initSize.x, initSize.y, true);
    }

    p = xml->Attribute("restrictSize");
    if (p)
    {
        ToolSet::splitString(p, ',', v4, true);
        minSize.x = v4.x;
        minSize.y = v4.z;
        maxSize.x = v4.y;
        maxSize.y = v4.w;
    }

    p = xml->Attribute("scale");
    if (p)
    {
        ToolSet::splitString(p, ',', v2);
        setScale(v2.x, v2.y);
    }

    p = xml->Attribute("skew");
    if (p)
    {
        ToolSet::splitString(p, ',', v2);
        setSkewX(v2.x);
        setSkewY(v2.y);
    }

    p = xml->Attribute("rotation");
    if (p)
        setRotation(atoi(p));

    p = xml->Attribute("pivot");
    if (p)
    {
        ToolSet::splitString(p, ',', v2);
        setPivot(v2.x, v2.y, xml->BoolAttribute("anchor"));
    }

    p = xml->Attribute("alpha");
    if (p)
        setAlpha((float)atof(p));

    p = xml->Attribute("touchable");
    if (p)
        setTouchable(strcmp(p, "true") == 0);

    p = xml->Attribute("visible");
    if (p)
        setVisible(strcmp(p, "true") == 0);

    p = xml->Attribute("grayed");
    if (p)
        setGrayed(strcmp(p, "true") == 0);

    p = xml->Attribute("tooltips");
    if (p)
        setTooltips(p);

    p = xml->Attribute("customData");
    if (p)
        _customData = Value(p);
}

void GObject::setup_AfterAdd(TXMLElement * xml)
{
    const char *p;

    p = xml->Attribute("group");
    if (p)
        _group = dynamic_cast<GGroup*>(_parent->getChildById(p));

    TXMLElement* exml = xml->FirstChildElement();
    while (exml)
    {
        int gearIndex = ToolSet::parseGearIndex(exml->Name());
        if (gearIndex != -1)
            getGear(gearIndex)->setup(exml);

        exml = exml->NextSiblingElement();
    }
}

void GObject::initDrag()
{
    if (_draggable)
    {
        addListener(UIEventType::TouchBegin, CALLBACK_1(GObject::onTouchBegin, this), EventTag(this));
        addListener(UIEventType::TouchMove, CALLBACK_1(GObject::onTouchMove, this), EventTag(this));
        addListener(UIEventType::TouchEnd, CALLBACK_1(GObject::onTouchEnd, this), EventTag(this));
    }
    else
    {
        removeListener(UIEventType::TouchBegin, EventTag(this));
        removeListener(UIEventType::TouchMove, EventTag(this));
        removeListener(UIEventType::TouchEnd, EventTag(this));
    }
}

void GObject::dragBegin(int touchId)
{
    if (_draggingObject != nullptr)
    {
        GObject* tmp = _draggingObject;
        _draggingObject->stopDrag();
        _draggingObject = nullptr;
        tmp->dispatchEvent(UIEventType::DragEnd);
    }

    sGlobalDragStart = StageInst->getTouchPosition(touchId);
    sGlobalRect = localToGlobal(VRectanglef(0, 0, _size.x, _size.y));

    _draggingObject = this;
    StageInst->addTouchMonitor(touchId, this);

    addListener(UIEventType::TouchMove, CALLBACK_1(GObject::onTouchMove, this), EventTag(this));
    addListener(UIEventType::TouchEnd, CALLBACK_1(GObject::onTouchEnd, this), EventTag(this));
}

void GObject::dragEnd()
{
    if (_draggingObject == this)
    {
        StageInst->removeTouchMonitor(this);
        _draggingObject = nullptr;
    }
}

void GObject::onTouchBegin(EventContext* context)
{
    _dragTouchStartPos = context->getInput()->getPosition();
    context->captureTouch();
}

void GObject::onTouchMove(EventContext* context)
{
    InputEvent* evt = context->getInput();

    if (_draggingObject != this && _draggable)
    {
        int sensitivity;
#ifdef CC_PLATFORM_PC
        sensitivity = UIConfig::clickDragSensitivity;
#else
        sensitivity = UIConfig::touchDragSensitivity;
#endif 
        if (std::abs(_dragTouchStartPos.x - evt->getPosition().x) < sensitivity
            && std::abs(_dragTouchStartPos.y - evt->getPosition().y) < sensitivity)
            return;

        if (!dispatchEvent(UIEventType::DragStart))
            dragBegin(evt->getTouchId());
        else
            context->uncaptureTouch();
    }

    if (_draggingObject == this)
    {
        float xx = evt->getPosition().x - sGlobalDragStart.x + sGlobalRect.m_vMin.x;
        float yy = evt->getPosition().y - sGlobalDragStart.y + sGlobalRect.m_vMin.y;

        if (_dragBounds != nullptr)
        {
            VRectanglef rect = UIRoot->localToGlobal(*_dragBounds);
            if (xx < rect.m_vMin.x)
                xx = rect.m_vMin.x;
            else if (xx + sGlobalRect.GetSizeX() > rect.m_vMax.x)
            {
                xx = rect.m_vMax.x - sGlobalRect.GetSizeX();
                if (xx < rect.m_vMin.x)
                    xx = rect.m_vMin.x;
            }

            if (yy < rect.m_vMin.y)
                yy = rect.m_vMin.y;
            else if (yy + sGlobalRect.GetSizeY() > rect.m_vMax.y)
            {
                yy = rect.m_vMax.y - sGlobalRect.GetSizeY();
                if (yy < rect.m_vMin.y)
                    yy = rect.m_vMin.y;
            }
        }

        hkvVec2 pt = _parent->globalToLocal(hkvVec2(xx, yy));

        sUpdateInDragging = true;
        setPosition((int)pt.x, (int)pt.y);
        sUpdateInDragging = false;

        dispatchEvent(UIEventType::DragMove);
    }
}

void GObject::onTouchEnd(EventContext* context)
{
    if (_draggingObject == this)
    {
        _draggingObject = nullptr;
        dispatchEvent(UIEventType::DragEnd);
    }
}

NS_FGUI_END