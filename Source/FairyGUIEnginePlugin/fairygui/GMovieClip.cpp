#include "GMovieClip.h"
#include "PackageItem.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN


GMovieClip::GMovieClip() :
    _content(nullptr)
{
    _sizeImplType = 1;
}

GMovieClip::~GMovieClip()
{
}

void GMovieClip::handleInit()
{
    _content = MovieClip::create();
    _content->retain();

    _displayObject = _content;
}

void GMovieClip::setPlaySettings(int start, int end, int times, int endAt, std::function<void()> completeCallback)
{
    _content->setPlaySettings(start, end, times, endAt, completeCallback);
}

void GMovieClip::setPlaying(bool value)
{
    _content->setPlaying(value);
}

void GMovieClip::setCurrentFrame(int value)
{
    _content->setCurrentFrame(value);
}

void GMovieClip::setColor(const VColorRef & value)
{
    _content->setColor(value);
}

void GMovieClip::constructFromResource()
{
    sourceSize.x = _packageItem->width;
    sourceSize.y = _packageItem->height;
    initSize = sourceSize;

    _content->setInterval(_packageItem->interval);
    _content->setRepeatDelay(_packageItem->repeatDelay);
    _content->setData(_packageItem->texture, _packageItem->frames, VRectanglef(0, 0, _packageItem->width, _packageItem->height));

    setSize(sourceSize.x, sourceSize.y);
}

void GMovieClip::setup_BeforeAdd(TXMLElement * xml)
{
    GObject::setup_BeforeAdd(xml);

    const char *p;

    p = xml->Attribute("frame");
    if (p)
        setCurrentFrame(atoi(p));

    p = xml->Attribute("playing");
    if (p)
        setPlaying(strcmp(p, "false") != 0);

    p = xml->Attribute("flip");
    if (p)
        setFlip(ToolSet::parseFlipType(p));

    p = xml->Attribute("color");
    if (p)
        setColor(ToolSet::convertFromHtmlColor(p));
}

NS_FGUI_END