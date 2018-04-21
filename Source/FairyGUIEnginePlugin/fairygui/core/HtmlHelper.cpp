#include "HtmlHelper.h"
#include "RichTextField.h"
#include "DisplayObject.h"
#include "GLoader.h"
#include "UIObjectFactory.h"
#include "UIPackage.h"
#include "SelectionShape.h"

NS_FGUI_BEGIN

bool HtmlParseOptions::defaultLinkUnderline = true;
VColorRef HtmlParseOptions::defaultLinkColor = VColorRef(0x3A, 0x67, 0xCC, 0xFF);
VColorRef HtmlParseOptions::defaultLinkBgColor = VColorRef(0, 0, 0, 0);
VColorRef HtmlParseOptions::defaultLinkHoverBgColor = VColorRef(0, 0, 0, 0);

HtmlParseOptions::HtmlParseOptions()
{
    linkUnderline = defaultLinkUnderline;
    linkColor = defaultLinkColor;
    linkBgColor = defaultLinkBgColor;
    linkHoverBgColor = defaultLinkHoverBgColor;
}

HtmlElement::HtmlElement(Type type) :
    width(0),
    height(0),
    status(0),
    position(0, 0),
    space(0),
    link(nullptr),
    htmlObject(nullptr)
{
    this->type = type;
};

HtmlElement::~HtmlElement()
{
    if (htmlObject != nullptr)
    {
        htmlObject->remove();
        delete htmlObject;
    }
}

void HtmlElement::createObject(RichTextField* textField)
{
    if (type == Type::IMAGE)
        htmlObject = new HtmlImage();
    else if (type == Type::LINK)
        htmlObject = new HtmlLink();

    if (htmlObject != nullptr)
        htmlObject->create(textField, this);
}

HtmlImage::HtmlImage()
{
    _loader = dynamic_cast<GLoader*>(UIObjectFactory::newObject("loader"));
    _loader->setFill(LoaderFillType::SCALE_FREE);
    _loader->setTouchable(false);
    _loader->retain();
}

HtmlImage::~HtmlImage()
{
    CC_SAFE_RELEASE(_loader);
}

float HtmlImage::getWidth()
{
    return _loader->getWidth();
}

float HtmlImage::getHeight()
{
    return _loader->getHeight();
}

void HtmlImage::create(RichTextField* owner, HtmlElement* element)
{
    _owner = owner;
    _element = element;
    _loader->setURL(element->text);
    _loader->setSize(element->width, element->height);
}

void HtmlImage::setPosition(float x, float y)
{
    _loader->setPosition(x, y);
}

void HtmlImage::add()
{
    _owner->addChild(_loader->displayObject());
}

void HtmlImage::remove()
{
    if (_loader->displayObject()->getParent() != nullptr)
        _owner->removeChild(_loader->displayObject());
}

void HtmlImage::release()
{
    _loader->removeAllListeners();

    _loader->setURL("");
    _owner = nullptr;
    _element = nullptr;
}

//------------------

HtmlLink::HtmlLink()
{
    _shape = SelectionShape::create();
    _shape->retain();
    _shape->setColor(VColorRef(0, 0, 0, 0));
    _shape->addListener(UIEventType::Click, CALLBACK_1(HtmlLink::onClick, this));
}

HtmlLink::~HtmlLink()
{
    CC_SAFE_RELEASE(_shape);
}

float HtmlLink::getWidth()
{
    return 0;
}

float HtmlLink::getHeight()
{
    return 0;
}

void HtmlLink::create(RichTextField* owner, HtmlElement* element)
{
    _owner = owner;
    _element = element;
}

void HtmlLink::setArea(int startLine, float startCharX, int endLine, float endCharX)
{
    std::vector<VRectanglef> rects;
    if (startLine == endLine && startCharX > endCharX)
    {
        float tmp = startCharX;
        startCharX = endCharX;
        endCharX = tmp;
    }
    _owner->getTextField()->getLinesShape(startLine, startCharX, endLine, endCharX, true, rects);
    _shape->setRects(rects);
}

void HtmlLink::setPosition(float x, float y)
{
    _shape->setPosition(x, y);
}

void HtmlLink::add()
{
    _owner->addChildAt(_shape, 0);
}

void HtmlLink::remove()
{
    if (_shape->getParent() != nullptr)
        _owner->removeChild(_shape);
}

void HtmlLink::release()
{
    _shape->removeAllListeners();

    _owner = nullptr;
    _element = nullptr;
}

void HtmlLink::onClick(EventContext * context)
{
    _owner->bubbleEvent(UIEventType::ClickLink, nullptr, Value(_element->text));
}

NS_FGUI_END