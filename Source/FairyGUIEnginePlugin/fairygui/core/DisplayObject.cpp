#include "DisplayObject.h"
#include "HitTest.h"
#include "FGUIManager.h"
#include "RenderContext.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

static void skewMatrix(hkvMat4& matrix, float skewX, float skewY)
{
    float sinX = hkvMath::sinDeg(skewX);
    float cosX = hkvMath::cosDeg(skewX);
    float sinY = hkvMath::sinDeg(skewY);
    float cosY = hkvMath::cosDeg(skewY);

    float m00 = matrix.m_Column[0][0] * cosY - matrix.m_Column[0][1] * sinX;
    float m10 = matrix.m_Column[0][0] * sinY + matrix.m_Column[0][1] * cosX;
    float m01 = matrix.m_Column[1][0] * cosY - matrix.m_Column[1][1] * sinX;
    float m11 = matrix.m_Column[1][0] * sinY + matrix.m_Column[1][1] * cosX;
    float m02 = matrix.m_Column[2][0] * cosY - matrix.m_Column[2][1] * sinX;
    float m12 = matrix.m_Column[2][0] * sinY + matrix.m_Column[2][1] * cosX;

    matrix.m_Column[0][0] = m00;
    matrix.m_Column[0][1] = m10;
    matrix.m_Column[1][0] = m01;
    matrix.m_Column[1][1] = m11;
    matrix.m_Column[2][0] = m02;
    matrix.m_Column[2][1] = m12;
}

DisplayObject::DisplayObject() :
    _parent(nullptr),
    _graphics(nullptr),
    _position(0, 0),
    _rotation(0, 0, 0),
    _scale(1, 1),
    _skew(0, 0),
    _contentRect(0, 0, 0, 0),
    _pivot(0, 0),
    _pivotOffset(0, 0),
    _visible(true),
    _touchable(true),
    _touchDisabled(false),
    _alpha(1),
    _grayed(false),
    _requireUpdateMesh(false),
    _outlineChanged(true),
    _matrixVersion(0),
    _parentMatrixVersion(0),
    _opaque(false),
    _touchChildren(true),
    _clipRect(nullptr),
    _hitArea(nullptr)
{
}

DisplayObject::~DisplayObject()
{
    removeFromParent();
    CC_SAFE_DELETE(_graphics);

    ssize_t cnt = _children.size();
    if (cnt > 0)
    {
        for (ssize_t i = 0; i < cnt; i++)
            _children.at(i)->_parent = nullptr;
        _children.clear();
    }

    CC_SAFE_DELETE(_hitArea);
    CC_SAFE_DELETE(_clipRect);
}

bool DisplayObject::init()
{
    return true;
}

void DisplayObject::setX(float value)
{
    _position.x = value;
    _outlineChanged = true;
}

void DisplayObject::setY(float value)
{
    _position.y = value;
    _outlineChanged = true;
}

void DisplayObject::setPosition(float xv, float yv)
{
    _position.x = xv;
    _position.y = yv;
    _outlineChanged = true;
}

hkvVec2 DisplayObject::getLocation()const
{
    hkvVec2 pos = _position;
    pos.x += _pivotOffset.x;
    pos.y += _pivotOffset.y;
    return pos;
}

void DisplayObject::setLocation(float xv, float yv)
{
    setPosition(xv - _pivotOffset.x, yv - _pivotOffset.y);
}

float DisplayObject::getWidth()
{
    ensureSizeCorrect();
    return _contentRect.m_vMax.x;
}

void DisplayObject::setWidth(float value)
{
    if (!hkvMath::isFloatEqual(value, _contentRect.m_vMax.x))
    {
        _contentRect.m_vMax.x = value;
        onSizeChanged(true, false);
    }
}

float DisplayObject::getHeight()
{
    ensureSizeCorrect();
    return _contentRect.m_vMax.y;
}

void DisplayObject::setHeight(float value)
{
    if (!hkvMath::isFloatEqual(value, _contentRect.m_vMax.y))
    {
        _contentRect.m_vMax.y = value;
        onSizeChanged(true, false);
    }
}

const hkvVec2& DisplayObject::getSize()
{
    ensureSizeCorrect();
    return _contentRect.m_vMax;
}

void DisplayObject::setSize(float wv, float hv)
{
    bool wc = !hkvMath::isFloatEqual(wv, _contentRect.m_vMax.x);
    bool hc = !hkvMath::isFloatEqual(hv, _contentRect.m_vMax.y);

    if (wc || hc)
    {
        _contentRect.m_vMax.x = wv;
        _contentRect.m_vMax.y = hv;
        onSizeChanged(wc, hc);
    }
}

void DisplayObject::ensureSizeCorrect()
{
}

void DisplayObject::onSizeChanged(bool widthChanged, bool heightChanged)
{
    applyPivot();
    //_paintingFlag = 1;
    if (_graphics != nullptr)
        _requireUpdateMesh = true;
    _outlineChanged = true;
}

void DisplayObject::setScaleX(float value)
{
    _scale.x = value;
    _outlineChanged = true;
    applyPivot();
}

void DisplayObject::setScaleY(float value)
{
    _scale.y = value;
    _outlineChanged = true;
    applyPivot();
}

void DisplayObject::setScale(float xv, float yv)
{
    _scale.x = xv;
    _scale.y = yv;
    _outlineChanged = true;
    applyPivot();
}

void DisplayObject::setSkew(float xv, float yv)
{
    _skew.x = xv;
    _skew.y = yv;
    _outlineChanged = true;
    applyPivot();
}

void DisplayObject::setPivot(float xv, float yv)
{
    hkvVec2 deltaPivot((xv - _pivot.x) * _contentRect.GetSizeX(), (yv - _pivot.y) * _contentRect.GetSizeY());
    hkvVec2 oldOffset = _pivotOffset;

    _pivot.x = xv;
    _pivot.y = yv;
    updatePivotOffset();
    _position += oldOffset - _pivotOffset + deltaPivot;
    _outlineChanged = true;
}

void DisplayObject::updatePivotOffset()
{
    float px = _pivot.x * _contentRect.GetSizeX();
    float py = _pivot.y * _contentRect.GetSizeY();

    hkvMat4 rotMatrix;
    rotMatrix.setFromEulerAngles(_rotation.x, _rotation.y, _rotation.z);

    hkvMat4 scaleMatrix;
    scaleMatrix.setScalingMatrix(_scale.getAsVec3(1));

    hkvMat4 matrix = rotMatrix * scaleMatrix;
    if (_skew.x != 0 || _skew.y != 0)
        skewMatrix(matrix, _skew.x, _skew.y);

    _pivotOffset = matrix.transformPosition(hkvVec3(px, py, 0)).getAsVec2();
}

void DisplayObject::applyPivot()
{
    if (_pivot.x != 0 || _pivot.y != 0)
    {
        hkvVec2 oldOffset = _pivotOffset;

        updatePivotOffset();
        _position += oldOffset - _pivotOffset;
        _outlineChanged = true;
    }
}

void DisplayObject::setAlpha(float value)
{
    _alpha = value;
}

void DisplayObject::setVisible(bool value)
{
    _visible = value;
}

void DisplayObject::setRotation(float value)
{
    _rotation.z = value;
    _outlineChanged = true;
    applyPivot();
}

void DisplayObject::setTouchable(bool value)
{
    _touchable = value;
}

void DisplayObject::setGrayed(bool value)
{
    if (_grayed != value)
    {
        _grayed = value;
    }
}

void DisplayObject::validateMatrix(bool checkParent)
{
    if (_parent != nullptr)
    {
        if (checkParent)
            _parent->validateMatrix(checkParent);
        if (_parentMatrixVersion != _parent->_matrixVersion)
        {
            _outlineChanged = true;
            _parentMatrixVersion = _parent->_matrixVersion;
        }
    }

    if (_outlineChanged)
    {
        _outlineChanged = false;
        _matrixVersion++;

        hkvMat4 transMatrix;
        transMatrix.setTranslationMatrix(_position.getAsVec3(0));

        hkvMat4 rotMatrix;
        rotMatrix.setFromEulerAngles(_rotation.x, _rotation.y, _rotation.z);

        hkvMat4 scaleMatrix;
        scaleMatrix.setScalingMatrix(_scale.getAsVec3(1));

        _localToWorldMatrix = transMatrix * rotMatrix * scaleMatrix;
        if (_skew.x != 0 || _skew.y != 0)
            skewMatrix(_localToWorldMatrix, _skew.x, _skew.y);
        if (_parent != nullptr)
            _localToWorldMatrix = _parent->_localToWorldMatrix * _localToWorldMatrix;
    }
}

const hkvMat4& DisplayObject::getLocalToWorldMatrix()
{
    validateMatrix(true);
    return _localToWorldMatrix;
}

VRectanglef DisplayObject::getBounds(DisplayObject* targetSpace)
{
    ensureSizeCorrect();

    if (targetSpace == this || _contentRect.GetSizeX() == 0 || _contentRect.GetSizeY() == 0) // optimization
    {
        return _contentRect;
    }
    else if (targetSpace == _parent && _rotation.z == 0)
    {
        return VRectanglef(_position.x, _position.y, _contentRect.GetSizeX() * _scale.x, _contentRect.GetSizeY() * _scale.y);
    }
    else
        return transformRect(_contentRect, targetSpace);
}

DisplayObject* DisplayObject::internalHitTest(HitTestContext* context)
{
    if (!_visible || (context->forTouch && (!_touchable || _touchDisabled)))
        return nullptr;

    return hitTest(context);
}

DisplayObject* DisplayObject::internalHitTestMask(HitTestContext* context)
{
    if (_visible)
        return hitTest(context);
    else
        return nullptr;
}

DisplayObject* DisplayObject::hitTest(const hkvVec2& stagePoint, bool forTouch)
{
    HitTestContext context;
    context.screenPoint = stagePoint;
    context.forTouch = forTouch;

    return hitTest(&context);
}

DisplayObject* DisplayObject::hitTest(HitTestContext* context)
{
    if (_scale.x == 0 || _scale.y == 0)
        return nullptr;

    hkvVec2 localPoint = globalToLocal(context->screenPoint);
    if (_hitArea != nullptr)
    {
        if (!_hitArea->hitTest(context, this, localPoint))
            return nullptr;
    }
    else
    {
        if (_clipRect != nullptr && !_clipRect->IsInside(localPoint))
            return nullptr;
    }

    DisplayObject* target = nullptr;
    if (!_children.empty() && _touchChildren)
    {
        ssize_t count = _children.size();
        for (ssize_t i = count - 1; i >= 0; --i) // front to back!
        {
            DisplayObject* child = _children.at(i);
            target = child->internalHitTest(context);
            if (target != nullptr)
                break;
        }
    }

    if (target == nullptr && _opaque && (_hitArea != nullptr || _contentRect.IsInside(localPoint)))
        target = this;

    return target;
}

hkvVec2 DisplayObject::globalToLocal(const hkvVec2& point)
{
    hkvMat4 mat = getLocalToWorldMatrix().getInverse();
    return mat.transformPosition(point.getAsVec3(0)).getAsVec2();
}

hkvVec2 DisplayObject::localToGlobal(const hkvVec2& point)
{
    return getLocalToWorldMatrix().transformPosition(point.getAsVec3(0)).getAsVec2();
}

hkvVec2 DisplayObject::transformPoint(const hkvVec2&  point, DisplayObject* targetSpace)
{
    if (targetSpace == nullptr)
        targetSpace = StageInst;

    if (targetSpace == this)
        return point;

    hkvVec3 vec3 = getLocalToWorldMatrix().transformPosition(point.getAsVec3(0));
    return targetSpace->getLocalToWorldMatrix().getInverse().transformPosition(vec3).getAsVec2();
}

VRectanglef DisplayObject::transformRect(const VRectanglef& rect, DisplayObject* targetSpace)
{
    if (targetSpace == nullptr)
        targetSpace = StageInst;

    if (targetSpace == this)
        return rect;

    if (targetSpace == _parent && _rotation.z == 0) // optimization
    {
        return VRectanglef((_position.x + rect.m_vMin.x) * _scale.x, (_position.y + rect.m_vMin.y) * _scale.y,
            rect.m_vMax.x * _scale.x, rect.m_vMax.y * _scale.y);
    }
    else
    {
        validateMatrix(true);

        hkvMat4 mat = targetSpace->getLocalToWorldMatrix().getInverse();
        return ToolSet::transformRect(rect, _localToWorldMatrix, mat);
    }
}

void DisplayObject::removeFromParent()
{
    if (_parent != nullptr)
        _parent->removeChild(this);
}

bool DisplayObject::onStage() const
{
    DisplayObject* currentObject = const_cast<DisplayObject*>(this);
    while (currentObject->_parent != nullptr)
        currentObject = currentObject->_parent;
    return dynamic_cast<Stage*>(currentObject) != nullptr;
}


DisplayObject* DisplayObject::addChild(DisplayObject* child)
{
    addChildAt(child, (int)_children.size());
    return child;
}

DisplayObject* DisplayObject::addChildAt(DisplayObject* child, int index)
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
        child->_parentMatrixVersion = 0;

        ssize_t cnt = _children.size();
        if (index == cnt)
            _children.pushBack(child);
        else
            _children.insert(index, child);

        child->release();

        if (onStage())
        {
            if (child->_children.empty())
                child->dispatchEvent(UIEventType::AddedToStage);
            else
                child->broadcastEvent(UIEventType::AddedToStage);
        }
    }
    return child;
}

void DisplayObject::removeChild(DisplayObject* child)
{
    CCASSERT(child != nullptr, "Argument must be non-nil");

    int childIndex = (int)_children.getIndex(child);
    if (childIndex != -1)
        removeChildAt(childIndex);
}

void DisplayObject::removeChildAt(int index)
{
    DisplayObject* child = _children.at(index);

    if (onStage())
    {
        if (child->_children.empty())
            child->dispatchEvent(UIEventType::RemoveFromStage);
        else
            child->broadcastEvent(UIEventType::RemoveFromStage);
    }

    child->_parent = nullptr;
    _children.erase(index);
}

void DisplayObject::removeChildren(int beginIndex, int endIndex)
{
    if (endIndex < 0 || endIndex >= (int)_children.size())
        endIndex = (int)_children.size() - 1;

    for (int i = beginIndex; i <= endIndex; ++i)
        removeChildAt(beginIndex);
}

DisplayObject* DisplayObject::getChildAt(int index) const
{
    return _children.at(index);
}

DisplayObject* DisplayObject::getChild(const std::string& name) const
{
    for (auto &child : _children)
    {
        if (child->name.compare(name) == 0)
            return child;
    }

    return nullptr;
}

int DisplayObject::getChildIndex(const DisplayObject* child) const
{
    CCASSERT(child != nullptr, "Argument must be non-nil");

    return _children.getIndex(const_cast<DisplayObject*>(child));
}

void DisplayObject::setChildIndex(DisplayObject* child, int index)
{
    CCASSERT(child != nullptr, "Argument must be non-nil");

    int oldIndex = (int)_children.getIndex(child);
    CCASSERT(oldIndex != -1, "Not a child of this container");

    child->retain();
    _children.erase(oldIndex);
    if (index >= (int)_children.size())
        _children.pushBack(child);
    else
        _children.insert(index, child);
    child->release();
}

void DisplayObject::swapChildren(DisplayObject* child1, DisplayObject* child2)
{
    CCASSERT(child1 != nullptr, "Argument1 must be non-nil");
    CCASSERT(child2 != nullptr, "Argument2 must be non-nil");

    int index1 = (int)_children.getIndex(child1);
    int index2 = (int)_children.getIndex(child2);

    CCASSERT(index1 != -1, "Not a child of this container");
    CCASSERT(index2 != -1, "Not a child of this container");

    swapChildrenAt(index1, index2);
}

void DisplayObject::swapChildrenAt(int index1, int index2)
{
    DisplayObject* child1 = _children.at(index1);
    DisplayObject* child2 = _children.at(index2);

    setChildIndex(child1, index2);
    setChildIndex(child2, index1);
}

int DisplayObject::numChildren() const
{
    return (int)_children.size();
}

bool DisplayObject::isAncestorOf(const DisplayObject * obj) const
{
    if (obj == nullptr)
        return false;

    DisplayObject* p = obj->_parent;
    while (p != nullptr)
    {
        if (p == this)
            return true;

        p = p->_parent;
    }
    return false;
}

const VRectanglef & DisplayObject::getClipRect() const
{
    if (_clipRect)
        return *_clipRect;
    else
        return EMPTY_RECT;
}

void DisplayObject::setClipRect(const VRectanglef& value)
{
    if (value.GetSizeX() > 0 && value.GetSizeY() > 0)
    {
        if (!_clipRect)
            _clipRect = new VRectanglef();
        *_clipRect = value;
    }
    else
    {
        CC_SAFE_DELETE(_clipRect);
    }
}

void DisplayObject::update(float dt)
{
    validateMatrix(false);

    for (auto &child : _children)
    {
        if (child->_visible)
            child->update(dt);
    }
}

void DisplayObject::onRender(RenderContext* context)
{
    if (_graphics != nullptr)
        _graphics->render(context, _localToWorldMatrix, _matrixVersion, _alpha);

    if (_clipRect != nullptr)
    {
        hkvMat4 mat;
        mat.setIdentity();
        context->enterClipping(ToolSet::transformRect(*_clipRect, _localToWorldMatrix, mat));
    }

    int cnt = (int)_children.size();
    if (cnt > 0)
    {
        float savedAlpha = context->alpha;
        context->alpha *= _alpha;
        bool savedGrayed = context->grayed;
        context->grayed = context->grayed || _grayed;

        for (int i = 0; i < cnt; i++)
        {
            DisplayObject* child = _children.at(i);
            if (child->_visible)
                child->onRender(context);
        }

        if (_clipRect != nullptr)
            context->leaveClipping();

        context->alpha = savedAlpha;
        context->grayed = savedGrayed;
    }
}

NS_FGUI_END