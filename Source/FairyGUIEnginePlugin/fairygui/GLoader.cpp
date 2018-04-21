#include "GLoader.h"
#include "UIPackage.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

GLoader::GLoader() :
    _autoSize(false),
    _align(AlignType::LEFT),
    _verticalAlign(VertAlignType::TOP),
    _fill(LoaderFillType::NONE),
    _updatingLayout(false),
    _contentItem(nullptr),
    _contentStatus(0),
    _content(nullptr)
{
}

GLoader::~GLoader()
{
    CC_SAFE_RELEASE(_content);
}

void GLoader::handleInit()
{
    _content = MovieClip::create();
    _content->retain();

    _displayObject = DisplayObject::create();
    _displayObject->retain();
    _displayObject->addChild(_content);
}

void GLoader::setURL(const std::string & value)
{
    if (_url.compare(value) == 0)
        return;

    _url = value;
    loadContent();
    updateGear(7);
}

void GLoader::setAlign(AlignType value)
{
    if (_align != value)
    {
        _align = value;
        updateLayout();
    }
}

void GLoader::setVerticalAlign(VertAlignType value)
{
    if (_verticalAlign != value)
    {
        _verticalAlign = value;
        updateLayout();
    }
}

void GLoader::setAutoSize(bool value)
{
    if (_autoSize != value)
    {
        _autoSize = value;
        updateLayout();
    }
}

void GLoader::setFill(LoaderFillType value)
{
    if (_fill != value)
    {
        _fill = value;
        updateLayout();
    }
}

void GLoader::setColor(const VColorRef & value)
{
    _content->setColor(value);
}

void GLoader::setPlaying(bool value)
{
    _content->setPlaying(value);
    updateGear(5);
}

void GLoader::setCurrentFrame(int value)
{
    _content->setCurrentFrame(value);
    updateGear(5);
}

void GLoader::loadContent()
{
    clearContent();

    if (_url.length() == 0)
        return;

    if (_url.compare(0, 5, "ui://") == 0)
        loadFromPackage();
    else
    {
        _contentStatus = 3;
        loadExternal();
    }
}

void GLoader::loadFromPackage()
{
    _contentItem = UIPackage::getItemByURL(_url);

    if (_contentItem != nullptr)
    {
        _contentItem->load();

        if (_contentItem->type == PackageItemType::IMAGE)
        {
            _contentStatus = 1;
            _contentSourceSize.x = _contentItem->width;
            _contentSourceSize.y = _contentItem->height;
            _content->setTexture(_contentItem->texture);
            if (_contentItem->scale9Grid)
                _content->setScale9Grid(*_contentItem->scale9Grid);
            updateLayout();
        }
        else if (_contentItem->type == PackageItemType::MOVIECLIP)
        {
            _contentStatus = 2;
            _contentSourceSize.x = _contentItem->width;
            _contentSourceSize.y = _contentItem->height;
            _content->setInterval(_contentItem->interval);
            _content->setRepeatDelay(_contentItem->repeatDelay);
            _content->setData(_contentItem->texture, _contentItem->frames, VRectanglef(0, 0, _contentItem->width, _contentItem->height));
            updateLayout();
        }
        else
        {
            if (_autoSize)
                setSize(_contentItem->width, _contentItem->height);

            setErrorState();
        }
    }
    else
        setErrorState();
}

void GLoader::loadExternal()
{
    VTextureObject* tex = Vision::TextureManager.Load2DTexture(_url.c_str(), VTM_FLAG_DEFAULT_NON_MIPMAPPED);
    if (tex)
        onExternalLoadSuccess(tex);
    else
        onExternalLoadFailed();
}

void GLoader::onExternalLoadSuccess(VTextureObject* texture)
{
    _contentStatus = 4;
    NTexture* ntex = new NTexture(texture);
    _content->setTexture(ntex);
    ntex->release();
    _contentSourceSize.x = texture->GetTextureWidth();
    _contentSourceSize.y = texture->GetTextureHeight();
    updateLayout();
}

void GLoader::onExternalLoadFailed()
{
    setErrorState();
}

void GLoader::clearContent()
{
    clearErrorState();

    _content->clear();
    _contentItem = nullptr;

    _contentStatus = 0;
}

void GLoader::updateLayout()
{
    if (_contentStatus == 0)
    {
        if (_autoSize)
        {
            _updatingLayout = true;
            setSize(50, 30);
            _updatingLayout = false;
        }
        return;
    }

    _contentSize = _contentSourceSize;

    if (_autoSize)
    {
        _updatingLayout = true;
        if (_contentSize.x == 0)
            _contentSize.x = 50;
        if (_contentSize.y == 0)
            _contentSize.y = 30;
        setSize(_contentSize.x, _contentSize.y);
        _updatingLayout = false;

        if (_size == _contentSize)
        {
            _content->setScale(1, 1);
            _content->setPosition(0, 0);
            return;
        }
    }

    float sx = 1, sy = 1;
    if (_fill != LoaderFillType::NONE)
    {
        sx = _size.x / _contentSourceSize.x;
        sy = _size.y / _contentSourceSize.y;

        if (sx != 1 || sy != 1)
        {
            if (_fill == LoaderFillType::SCALE_MATCH_HEIGHT)
                sx = sy;
            else if (_fill == LoaderFillType::SCALE_MATCH_WIDTH)
                sy = sx;
            else if (_fill == LoaderFillType::SCALE)
            {
                if (sx > sy)
                    sx = sy;
                else
                    sy = sx;
            }
            else if (_fill == LoaderFillType::SCALE_NO_BORDER)
            {
                if (sx > sy)
                    sy = sx;
                else
                    sx = sy;
            }
            _contentSize.x = floor(_contentSourceSize.x * sx);
            _contentSize.y = floor(_contentSourceSize.y * sy);
        }
    }

    _content->setSize(_contentSourceSize.x, _contentSourceSize.y);
    _content->setScale(sx, sy);

    float nx;
    float ny;
    if (_align == AlignType::CENTER)
        nx = floor((_size.x - _contentSize.x) / 2);
    else if (_align == AlignType::RIGHT)
        nx = floor(_size.x - _contentSize.x);
    else
        nx = 0;
    if (_verticalAlign == VertAlignType::MIDDLE)
        ny = floor((_size.y - _contentSize.y) / 2);
    else if (_verticalAlign == VertAlignType::BOTTOM)
        nx = floor(_size.y - _contentSize.y);
    else
        ny = 0;

    _content->setPosition(nx, ny);
}

void GLoader::setErrorState()
{
}

void GLoader::clearErrorState()
{
}

void GLoader::handleSizeChanged()
{
    GObject::handleSizeChanged();

    if (!_updatingLayout)
        updateLayout();
}

void GLoader::handleGrayedChanged()
{
    _content->setGrayed(_grayed);
}

void GLoader::setup_BeforeAdd(TXMLElement * xml)
{
    GObject::setup_BeforeAdd(xml);

    const char*p;

    p = xml->Attribute("url");
    if (p)
        _url = p;

    p = xml->Attribute("align");
    if (p)
        _align = ToolSet::parseAlign(p);

    p = xml->Attribute("vAlign");
    if (p)
        _verticalAlign = ToolSet::parseVerticalAlign(p);

    p = xml->Attribute("fill");
    if (p)
        _fill = ToolSet::parseFillType(p);
        
    _autoSize = xml->BoolAttribute("autoSize");

    p = xml->Attribute("color");
    if (p)
        setColor(ToolSet::convertFromHtmlColor(p));

    p = xml->Attribute("frame");
    if (p)
        _content->setCurrentFrame(atoi(p));

    p = xml->Attribute("playing");
    if (p)
        _content->setPlaying(strcmp(p, "false") != 0);

    if (_url.length() > 0)
        loadContent();
}

NS_FGUI_END