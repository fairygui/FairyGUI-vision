#include "GImage.h"
#include "PackageItem.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

GImage::GImage() :
    _content(nullptr)
{
}

GImage::~GImage()
{
}

void GImage::handleInit()
{
    _content = Image::create();
    _content->retain();

    _displayObject = _content;
}

void GImage::setColor(const VColorRef & value)
{
    _content->setColor(value);
}

void GImage::constructFromResource()
{
    sourceSize.x = _packageItem->width;
    sourceSize.y = _packageItem->height;
    initSize = sourceSize;

    if (_packageItem->scale9Grid)
        _content->setScale9Grid(*_packageItem->scale9Grid);
    _content->setScaleByTile(_packageItem->scaleByTile);
    _content->setTileGridIndice(_packageItem->tileGridIndice);

    _content->setTexture(_packageItem->texture);

    setSize(sourceSize.x, sourceSize.y);
}

void GImage::setup_BeforeAdd(TXMLElement * xml)
{
    GObject::setup_BeforeAdd(xml);

    const char *p;

    p = xml->Attribute("flip");
    if (p)
        setFlip(ToolSet::parseFlipType(p));

    p = xml->Attribute("color");
    if (p)
        setColor(ToolSet::convertFromHtmlColor(p));
}

NS_FGUI_END