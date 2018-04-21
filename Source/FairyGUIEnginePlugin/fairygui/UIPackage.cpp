#include "UIPackage.h"
#include "UIObjectFactory.h"
#include "GComponent.h"
#include "FGUIManager.h"
#include "core/HitTest.h"
#include "core/BitmapFont.h"
#include "utils/ByteArray.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

const std::string UIPackage::URL_PREFIX = "ui://";
int UIPackage::_constructing = 0;

std::unordered_map<std::string, UIPackage*> UIPackage::_packageInstById;
std::unordered_map<std::string, UIPackage*> UIPackage::_packageInstByName;
std::vector<UIPackage*> UIPackage::_packageList;
std::unordered_map<std::string, ValueMap> UIPackage::_stringsSource;

struct AtlasSprite
{
    std::string atlas;
    VRectanglef rect;
    bool rotated;
};

UIPackage::UIPackage() :
    _loadingPackage(false)
{
}

UIPackage::~UIPackage()
{
    for (auto &it : _items)
        delete it;
    for (auto &it : _hitTestDatas)
        delete it.second;
}

UIPackage * UIPackage::getById(const std::string& id)
{
    auto it = _packageInstById.find(id);
    if (it != _packageInstById.end())
        return it->second;
    else
        return nullptr;
}

UIPackage * UIPackage::getByName(const std::string& name)
{
    auto it = _packageInstByName.find(name);
    if (it != _packageInstByName.end())
        return it->second;
    else
        return nullptr;
}

UIPackage * UIPackage::addPackage(const std::string& assetPath)
{
    auto it = _packageInstById.find(assetPath);
    if (it != _packageInstById.end())
        return it->second;

    UIPackage* pkg = new UIPackage();
    pkg->create(assetPath);

    _packageInstById[pkg->getId()] = pkg;
    _packageInstByName[pkg->getName()] = pkg;
    _packageInstById[assetPath] = pkg;
    _packageList.push_back(pkg);

    return pkg;
}

void UIPackage::removePackage(const std::string& packageIdOrName)
{
    UIPackage* pkg = UIPackage::getByName(packageIdOrName);
    if (!pkg)
        pkg = getById(packageIdOrName);

    if (pkg)
    {
        auto it = std::find(_packageList.cbegin(), _packageList.cend(), pkg);
        if (it != _packageList.cend())
            _packageList.erase(it);

        _packageInstById.erase(pkg->getId());
        _packageInstById.erase(pkg->_assetPath);
        _packageInstByName.erase(pkg->getName());

        delete pkg;
    }
    else
        CCLOGERROR("FairyGUI: invalid package name or id: %s", packageIdOrName.c_str());
}

void UIPackage::removeAllPackages()
{
    for (auto &it : _packageList)
        delete it;

    _packageInstById.clear();
    _packageInstByName.clear();
    _packageList.clear();
}


GObject* UIPackage::createObject(const std::string& pkgName, const std::string& resName)
{
    UIPackage* pkg = UIPackage::getByName(pkgName);
    if (pkg)
        return pkg->createObject(resName);
    else
        return nullptr;
}

GObject* UIPackage::createObjectFromURL(const std::string& url)
{
    PackageItem* pi = UIPackage::getItemByURL(url);
    if (pi)
        return pi->owner->createObject(pi);
    else
        return nullptr;
}

std::string UIPackage::getItemURL(const std::string& pkgName, const std::string& resName)
{
    UIPackage* pkg = UIPackage::getByName(pkgName);
    if (pkg)
    {
        PackageItem* pi = pkg->getItemByName(resName);
        if (pi)
            return URL_PREFIX + pkg->getId() + pi->id;
    }
    return "";
}

PackageItem * UIPackage::getItemByURL(const std::string& url)
{
    if (url.size() == 0)
        return nullptr;

    int pos1 = url.find('/');
    if (pos1 == -1)
        return nullptr;

    int pos2 = url.find('/', pos1 + 2);
    if (pos2 == -1)
    {
        if (url.size() > 13)
        {
            std::string pkgId = url.substr(5, 8);
            UIPackage* pkg = getById(pkgId);
            if (pkg != nullptr)
            {
                std::string srcId = url.substr(13);
                return pkg->getItem(srcId);
            }
        }
    }
    else
    {
        std::string pkgName = url.substr(pos1 + 2, pos2 - pos1 - 2);
        UIPackage* pkg = getByName(pkgName);
        if (pkg != nullptr)
        {
            std::string srcName = url.substr(pos2 + 1);
            return pkg->getItemByName(srcName);
        }
    }

    return nullptr;
}

std::string UIPackage::normalizeURL(const std::string& url)
{
    if (url.size() == 0)
        return url;

    int pos1 = url.find('/');
    if (pos1 == -1)
        return url;

    int pos2 = url.find('/', pos1 + 2);
    if (pos2 == -1)
        return url;
    else
    {
        std::string pkgName = url.substr(pos1 + 2, pos2 - pos1 - 2);
        std::string srcName = url.substr(pos2 + 1);
        return getItemURL(pkgName, srcName);
    }
}

PackageItem * UIPackage::getItem(const std::string & itemId)
{
    auto it = _itemsById.find(itemId);
    if (it != _itemsById.end())
        return it->second;
    else
        return nullptr;
}

PackageItem * UIPackage::getItemByName(const std::string & itemName)
{
    auto it = _itemsByName.find(itemName);
    if (it != _itemsByName.end())
        return it->second;
    else
        return nullptr;
}

void UIPackage::loadItem(const std::string& resName)
{
    PackageItem* pi = getItemByName(resName);
    if (pi)
        return loadItem(pi);
}

void UIPackage::loadItem(PackageItem * item)
{
    switch (item->type)
    {
    case PackageItemType::IMAGE:
        if (!item->decoded)
        {
            item->decoded = true;
            auto it = _sprites.find(item->id);
            if (it != _sprites.end())
            {
                AtlasSprite * sprite = it->second;
                item->texture = createSpriteTexture(sprite);
            }
            else
            {
                CCLOGWARN("FairyGUI: %s not found in %s", item->id, _name.c_str());
                item->texture = FGUIManager::GlobalManager().getWhiteTexture();
                item->texture->retain();
            }
        }
        break;

    case PackageItemType::ATLAS:
        if (!item->decoded)
        {
            item->decoded = true;
            loadAtlas(item);
        }
        break;

    case PackageItemType::SOUND:
        if (!item->decoded)
        {
            item->decoded = true;
            loadSound(item);
        }
        break;

    case PackageItemType::FONT:
        if (!item->decoded)
        {
            item->decoded = true;
            loadFont(item);
        }
        break;

    case PackageItemType::MOVIECLIP:
        if (!item->decoded)
        {
            item->decoded = true;
            loadMovieClip(item);
        }
        break;

    case PackageItemType::COMPONENT:
        if (!item->decoded)
        {
            item->decoded = true;
            loadComponent(item);
        }
        if (!_loadingPackage && !item->displayList)
        {
            loadComponentChildren(item);
            translateComponent(item);
        }
        break;
    default:
        break;
    }
}

PixelHitTestData * UIPackage::getPixelHitTestData(const std::string & itemId)
{
    auto it = _hitTestDatas.find(itemId);
    if (it != _hitTestDatas.end())
        return it->second;
    else
        return nullptr;
}


void UIPackage::setStringsSource(const char *xmlString, size_t nBytes)
{
    _stringsSource.clear();

    TXMLDocument* xml = new TXMLDocument();
    xml->Parse(xmlString, nBytes);

    TXMLElement* root = xml->RootElement();
    TXMLElement* ele = root->FirstChildElement("string");
    while (ele)
    {
        std::string key = ele->Attribute("name");
        std::string text = ele->GetText();
        size_t i = key.find("-");
        if (i == std::string::npos)
            continue;

        std::string key2 = key.substr(0, i);
        std::string key3 = key.substr(i + 1);
        ValueMap& col = _stringsSource[key2];
        col[key3] = text;

        ele = ele->NextSiblingElement("string");
    }

    delete xml;
}

void UIPackage::create(const std::string& assetPath)
{
    IVFileInStream* stream = VFileAccessManager::GetInstance()->Open((assetPath + ".bytes").c_str());
    if (stream == nullptr)
    {
        CCLOGERROR("FairyGUI: cannot load package from '%s'", assetPath.c_str());
        return;
    }
    hkvArray<char> buffer;
    buffer.SetSize(stream->GetSize());
    stream->Read(buffer.GetData(), buffer.GetSize());
    stream->Close();

    _assetNamePrefix = assetPath + "@";
    decodeDesc(buffer);

    loadPackage();
}

void UIPackage::decodeDesc(hkvArray<char>& buffer)
{
    ByteArray* ba = ByteArray::createWithBuffer((char *)buffer.GetData(), buffer.GetSize());

    size_t pos = ba->getLength() - 22;
    ba->setPosition(pos + 10);
    int entryCount = ba->readShort();
    ba->setPosition(pos + 16);
    pos = ba->readInt();

    for (int i = 0; i < entryCount; i++)
    {
        ba->setPosition(pos + 28);
        int len = ba->readUnsignedShort();
        int len2 = ba->readUnsignedShort() + ba->readUnsignedShort();

        ba->setPosition(pos + 46);
        std::string entryName = ba->readString(len);

        if (entryName[entryName.size() - 1] != '/' && entryName[entryName.size() - 1] != '\\') //not directory
        {
            ba->setPosition(pos + 20);
            int size = ba->readInt();
            ba->setPosition(pos + 42);
            int offset = ba->readInt() + 30 + len;

            if (size > 0)
            {
                hkvArray<char>* data = new hkvArray<char>();
                data->SetSize(size);
                memcpy(data->GetDataPointer(), buffer.GetData() + offset, size);
                _descPack[entryName] = data;
            }
        }

        pos += 46 + len + len2;
    }

    CC_SAFE_DELETE(ba);
}

void UIPackage::loadPackage()
{
    IVFileInStream* stream = VFileAccessManager::GetInstance()->Open((_assetNamePrefix + "sprites.bytes").c_str());
    if (stream == nullptr)
    {
        CCLOGERROR("FairyGUI: cannot load package from '%s'", _assetNamePrefix.c_str());
        return;
    }
    hkvArray<char> buffer;
    buffer.SetSize(stream->GetSize());
    stream->Read(buffer.GetData(), buffer.GetSize());
    stream->Close();

    _loadingPackage = true;

    std::vector<std::string> lines;
    std::vector<std::string> arr;
    std::string str;

    ToolSet::splitString(std::string(buffer.GetData(), buffer.GetSize()), '\n', lines);
    size_t cnt = lines.size();
    for (size_t i = 1; i < cnt; i++)
    {
        str = lines[i];
        if (str.size() == 0)
            continue;

        ToolSet::splitString(str, ' ', arr);
        AtlasSprite* sprite = new AtlasSprite();
        std::string itemId = arr[0];
        int binIndex = atoi(arr[1].c_str());
        if (binIndex >= 0)
            sprite->atlas = "atlas" + Value(binIndex).asString();
        else
        {
            size_t pos = itemId.find_first_of("_");
            if (pos == -1)
                sprite->atlas = "atlas_" + itemId;
            else
                sprite->atlas = "atlas_" + itemId.substr(0, pos);
        }
        sprite->rect.m_vMin.x = (float)atoi(arr[2].c_str());
        sprite->rect.m_vMin.y = (float)atoi(arr[3].c_str());
        sprite->rect.m_vMax.x = sprite->rect.m_vMin.x + atoi(arr[4].c_str());
        sprite->rect.m_vMax.y = sprite->rect.m_vMin.y + atoi(arr[5].c_str());
        sprite->rotated = arr[6] == "1";
        _sprites[itemId] = sprite;
    }

    stream = VFileAccessManager::GetInstance()->Open((_assetNamePrefix + "hittest.bytes").c_str());
    if (stream)
    {
        hkvArray<char> tmpBuffer;
        tmpBuffer.SetSize(stream->GetSize());
        stream->Read(tmpBuffer.GetData(), tmpBuffer.GetSize());
        stream->Close();

        ByteArray* ba = ByteArray::createWithBuffer(tmpBuffer.GetData(), tmpBuffer.GetSize(), false);
        ba->setEndian(ByteArray::ENDIAN_BIG);
        while (ba->getBytesAvailable())
        {
            PixelHitTestData* pht = new PixelHitTestData();
            _hitTestDatas[ba->readString()] = pht;
            pht->load(*ba);
        }
        delete ba;
    }

    hkvArray<char>* xmlData = _descPack["package.xml"];

    TXMLDocument* xml = new TXMLDocument();
    xml->Parse((const char*)xmlData->GetData(), xmlData->GetSize());

    TXMLElement* root = xml->RootElement();

    _id = root->Attribute("id");
    _name = root->Attribute("name");

    TXMLElement* rxml = root->FirstChildElement("resources");
    if (rxml == nullptr)
    {
        CCLOGERROR("FairyGUI: invalid package xml '%s'", _assetNamePrefix.c_str());
        return;
    }

    PackageItem* pi;
    TXMLElement* cxml = rxml->FirstChildElement();
    hkvVec2 v2;
    hkvVec4 v4;
    const char *p;
    while (cxml)
    {
        pi = new PackageItem();
        pi->owner = this;
        pi->type = ToolSet::parsePackageItemType(cxml->Name());
        pi->id = cxml->Attribute("id");

        p = cxml->Attribute("name");
        if (p)
            pi->name = p;

        pi->exported = cxml->BoolAttribute("exported");

        p = cxml->Attribute("file");
        if (p)
            pi->file = p;

        p = cxml->Attribute("size");
        if (p)
        {
            ToolSet::splitString(p, ',', v2, true);
            pi->width = (int)v2.x;
            pi->height = (int)v2.y;
        }
        switch (pi->type)
        {
        case PackageItemType::IMAGE:
        {
            p = cxml->Attribute("scale");
            if (p)
            {
                if (strcmp(p, "9grid") == 0)
                {
                    p = cxml->Attribute("scale9grid");
                    if (p)
                    {
                        ToolSet::splitString(p, ',', v4, true);
                        pi->scale9Grid = new VRectanglef();
                        pi->scale9Grid->m_vMin.x = v4.x;
                        pi->scale9Grid->m_vMin.y = v4.y;
                        pi->scale9Grid->m_vMax.x = v4.x + v4.z;
                        pi->scale9Grid->m_vMax.y = v4.y + v4.w;

                        pi->tileGridIndice = cxml->IntAttribute("gridTile");
                    }
                }
                else if (strcmp(p, "tile") == 0)
                    pi->scaleByTile = true;
            }
            break;
        }

        case PackageItemType::COMPONENT:
        {
            UIObjectFactory::resolvePackageItemExtension(pi);
            break;
        }

        default:
            break;
        }
        _items.push_back(pi);
        _itemsById[pi->id] = pi;
        if (!pi->name.empty())
            _itemsByName[pi->name] = pi;

        cxml = cxml->NextSiblingElement();
    }

    delete xml;

    for (auto &iter : _items)
        loadItem(iter);

    for (auto &iter : _descPack)
        delete iter.second;
    _descPack.clear();

    for (auto &iter : _sprites)
        delete iter.second;
    _sprites.clear();

    _loadingPackage = false;
}

NTexture* UIPackage::createSpriteTexture(AtlasSprite * sprite)
{
    PackageItem* atlasItem = getItem(sprite->atlas);
    NTexture* atlasTexture;
    if (atlasItem)
    {
        loadItem(atlasItem);
        atlasTexture = atlasItem->texture;
    }
    else
    {
        atlasTexture = FGUIManager::GlobalManager().getWhiteTexture();
        CCLOGWARN("FairyGUI: %s not found in %s", sprite->atlas.c_str(), _name.c_str());
    }

    return new NTexture(atlasTexture, sprite->rect, sprite->rotated);
}

void UIPackage::loadAtlas(PackageItem * item)
{
    std::string filePath = _assetNamePrefix + (!item->file.empty() ? item->file : item->id + ".png");
    VTextureObject* tex = Vision::TextureManager.Load2DTexture(filePath.c_str(), VTM_FLAG_NO_MIPMAPS | VTM_FLAG_NO_DOWNSCALE | VTM_FLAG_VERTEXTEXTURE);
    if (tex)
    {
        item->texture = new NTexture(tex);
    }
    else
    {
        item->texture = FGUIManager::GlobalManager().getWhiteTexture();
        item->texture->retain();
        CCLOGWARN("FairyGUI: texture '%s' not found in %s", filePath.c_str(), _name.c_str());
    }
}

void UIPackage::loadSound(PackageItem * item)
{
    std::string filePath = _assetNamePrefix + (!item->file.empty() ? item->file : item->id + ".mp3");
    VFmodSoundObject* soundObj = VFmodManager::GlobalManager().CreateSoundInstance(filePath.c_str(), VFMOD_RESOURCEFLAG_2D, VFMOD_FLAG_PAUSED | VFMOD_FLAG_NODISPOSE);
    if (soundObj)
    {
        item->sound = soundObj;
    }
    else
        CCLOGWARN("FairyGUI: texture '%s' not found in %s", filePath.c_str(), _name.c_str());
}

void UIPackage::loadMovieClip(PackageItem * item)
{
    hkvArray<char>* xmlData = _descPack[item->id + ".xml"];
    TXMLDocument* xml = new TXMLDocument();
    xml->Parse((const char*)xmlData->GetData(), xmlData->GetSize());

    TXMLElement* root = xml->RootElement();

    item->interval = root->FloatAttribute("interval") / 1000.0f;
    item->repeatDelay = root->FloatAttribute("repeatDelay") / 1000.0f;
    bool swing = root->BoolAttribute("swing");

    int frameCount = root->IntAttribute("frameCount");
    item->frames.SetSize(frameCount);

    int i = 0;
    std::string spriteId;
    AtlasSprite* sprite;
    const char* p;
    std::vector<std::string> arr;

    TXMLElement* framesEle = root->FirstChildElement("frames");
    TXMLElement* frameEle = framesEle->FirstChildElement("frame");
    while (frameEle)
    {
        MovieClip::Frame& frame = item->frames[i];
        ToolSet::splitString(frameEle->Attribute("rect"), ',', arr);
        frame.rect.m_vMin.x = atoi(arr[0].c_str());
        frame.rect.m_vMin.y = atoi(arr[1].c_str());
        frame.rect.m_vMax.x = atoi(arr[2].c_str()) + frame.rect.m_vMin.x;
        frame.rect.m_vMax.y = atoi(arr[3].c_str()) + frame.rect.m_vMin.y;
        frame.addDelay = frameEle->IntAttribute("addDelay") / 1000.0f;
        frame.uvRect.Set(0, 0, 0, 0);
        frame.rotated = false;

        p = frameEle->Attribute("sprite");
        if (p)
            spriteId = item->id + "_" + std::string(p);
        else if (frame.rect.GetSizeX() != 0)
            spriteId = item->id + "_" + Value(i).asString();
        else
            spriteId.clear();

        if (!spriteId.empty())
        {
            auto it = _sprites.find(spriteId);
            if (it != _sprites.end())
            {
                sprite = it->second;
                PackageItem* atlasItem = getItem(sprite->atlas);
                if (atlasItem)
                {
                    if (item->texture == nullptr)
                    {
                        item->texture = atlasItem->texture;
                        item->texture->retain();
                    }
                    frame.uvRect.Set(sprite->rect.m_vMin.x / item->texture->getWidth() * item->texture->getUVRect().GetSizeX(),
                        sprite->rect.m_vMin.y * item->texture->getUVRect().GetSizeY() / item->texture->getHeight(),
                        sprite->rect.m_vMax.x * item->texture->getUVRect().GetSizeX() / item->texture->getWidth(),
                        sprite->rect.m_vMax.y * item->texture->getUVRect().GetSizeY() / item->texture->getHeight());
                    frame.rotated = sprite->rotated;
                    if (frame.rotated)
                    {
                        float tmp = frame.uvRect.GetSizeX();
                        frame.uvRect.m_vMax.x = frame.uvRect.m_vMin.x + frame.uvRect.GetSizeY();
                        frame.uvRect.m_vMax.y = frame.uvRect.m_vMin.y + tmp;

                        tmp = frame.uvRect.GetSizeX();
                        frame.uvRect.m_vMax.x = frame.uvRect.m_vMin.x + frame.uvRect.GetSizeY();
                        frame.uvRect.m_vMax.y = frame.uvRect.m_vMin.y + tmp;
                    }
                }
            }
        }

        i++;
        frameEle = frameEle->NextSiblingElement("frame");
    }
    delete xml;
}

void UIPackage::loadFont(PackageItem * item)
{
    hkvArray<char>* fntData = _descPack[item->id + ".fnt"];

    FastSplitter lines, props, fields;
    lines.start((char*)fntData->GetData(), fntData->GetSize(), '\n');
    bool ttf = false;
    int size = 0;
    int xadvance = 0;
    bool scaleEnabled = false;
    bool colorEnabled = false;
    int lineHeight = 0;
    float texScaleX = 1;
    float texScaleY = 1;
    NTexture* mainTexture = nullptr;
    AtlasSprite* mainSprite = nullptr;
    char keyBuf[30];
    char valueBuf[50];

    item->bitmapFont = new BitmapFont(URL_PREFIX + _id + item->id);

    while (lines.next())
    {
        size_t len = lines.getTextLength();
        const char* line = lines.getText();
        if (len > 4 && memcmp(line, "info", 4) == 0)
        {
            props.start(line, len, ' ');
            while (props.next())
            {
                props.getKeyValuePair(keyBuf, sizeof(keyBuf), valueBuf, sizeof(valueBuf));

                if (strcmp(keyBuf, "face") == 0)
                {
                    ttf = true;
                    colorEnabled = true;

                    auto it = _sprites.find(item->id);
                    if (it != _sprites.end())
                    {
                        mainSprite = it->second;
                        PackageItem* atlasItem = getItem(mainSprite->atlas);
                        loadItem(atlasItem);
                        mainTexture = atlasItem->texture;
                        mainTexture->retain();
                        texScaleX = mainTexture->getUVRect().GetSizeX() / mainTexture->getWidth();
                        texScaleY = mainTexture->getUVRect().GetSizeY() / mainTexture->getHeight();
                    }
                }
                else if (strcmp(keyBuf, "size") == 0)
                    sscanf(valueBuf, "%d", &size);
                else if (strcmp(keyBuf, "resizable") == 0)
                    scaleEnabled = strcmp(valueBuf, "true") == 0;
                else if (strcmp(keyBuf, "colored") == 0)
                    colorEnabled = strcmp(valueBuf, "true") == 0;
            }

            if (size == 0)
                size = lineHeight;
            else if (lineHeight == 0)
                lineHeight = size;
        }
        else if (len > 6 && memcmp(line, "common", 6) == 0)
        {
            props.start(line, len, ' ');
            while (props.next())
            {
                props.getKeyValuePair(keyBuf, sizeof(keyBuf), valueBuf, sizeof(valueBuf));

                if (strcmp(keyBuf, "lineHeight") == 0)
                    sscanf(valueBuf, "%d", &lineHeight);

                if (strcmp(keyBuf, "xadvance") == 0)
                    sscanf(valueBuf, "%d", &xadvance);
            }
        }
        else if (len > 4 && memcmp(line, "char", 4) == 0)
        {
            BitmapFont::BMGlyph def;
            memset(&def, 0, sizeof(def));

            int bx = 0, by = 0;
            hkUint32 charId = 0;
            PackageItem* charImg = nullptr;

            props.start(line, len, ' ');
            while (props.next())
            {
                props.getKeyValuePair(keyBuf, sizeof(keyBuf), valueBuf, sizeof(valueBuf));

                if (strcmp(keyBuf, "id") == 0)
                    sscanf(valueBuf, "%d", &charId);
                else if (strcmp(keyBuf, "x") == 0)
                    sscanf(valueBuf, "%d", &bx);
                else if (strcmp(keyBuf, "y") == 0)
                    sscanf(valueBuf, "%d", &by);
                else if (strcmp(keyBuf, "xoffset") == 0)
                    sscanf(valueBuf, "%d", &def.offsetX);
                else if (strcmp(keyBuf, "yoffset") == 0)
                    sscanf(valueBuf, "%d", &def.offsetY);
                else if (strcmp(keyBuf, "width") == 0)
                    sscanf(valueBuf, "%d", &def.width);
                else if (strcmp(keyBuf, "height") == 0)
                    sscanf(valueBuf, "%d", &def.height);
                else if (strcmp(keyBuf, "xadvance") == 0)
                    sscanf(valueBuf, "%d", &def.advance);
                else if (!ttf && strcmp(keyBuf, "img") == 0)
                    charImg = getItem(valueBuf);
            }

            if (ttf)
            {
                def.uv.Set((float)((bx + mainSprite->rect.m_vMin.x) * texScaleX), (float)((by + mainSprite->rect.m_vMin.y) * texScaleY),
                    (float)((bx + def.width + mainSprite->rect.m_vMin.x) * texScaleX), (float)((by + def.height + mainSprite->rect.m_vMin.y) * texScaleY));
                if (mainSprite->rotated)
                {

                }

                def.lineHeight = lineHeight;
            }
            else if (charImg)
            {
                loadItem(charImg);
                def.uv = charImg->texture->getUVRect();
                if (charImg->texture->isRotated())
                {
                }
                def.width = charImg->texture->getWidth();
                def.height = charImg->texture->getHeight();
                if (mainTexture == nullptr)
                    mainTexture = new NTexture(charImg->texture->getNativeTexture());

                if (def.advance == 0)
                {
                    if (xadvance == 0)
                        def.advance = def.offsetX + def.width;
                    else
                        def.advance = xadvance;
                }

                def.lineHeight = def.offsetY < 0 ? def.height : (def.offsetY + def.height);
                if (def.lineHeight < size)
                    def.lineHeight = size;

                if (lineHeight < def.lineHeight)
                    lineHeight = def.lineHeight;
            }

            if (size == 0)
                size = def.height;

            item->bitmapFont->chars[charId] = def;
        }
    }

    item->bitmapFont->setTexture(mainTexture);
    item->bitmapFont->size = size;
    item->bitmapFont->lineHeight = lineHeight;
    item->bitmapFont->scaleEnabled = scaleEnabled;
    item->bitmapFont->colorEnabled = colorEnabled;
}

void UIPackage::loadComponent(PackageItem * item)
{
    hkvArray<char>* xmlData = _descPack[item->id + ".xml"];
    TXMLDocument* doc = new TXMLDocument();
    doc->Parse((const char*)xmlData->GetData(), xmlData->GetSize());
    item->componentData = doc;
}

void UIPackage::loadComponentChildren(PackageItem * item)
{
    TXMLElement* listNode = item->componentData->RootElement()->FirstChildElement("displayList");
    item->displayList = new std::vector<DisplayListItem*>();
    const char *p;
    if (listNode)
    {
        TXMLElement* cxml = listNode->FirstChildElement();
        DisplayListItem* di;
        while (cxml)
        {
            p = cxml->Attribute("src");
            if (p)
            {
                std::string src = p;
                std::string pkgId = (p = cxml->Attribute("pkg")) ? p : "";
                UIPackage* pkg;
                if (!pkgId.empty() && pkgId.compare(item->owner->getId()) != 0)
                    pkg = UIPackage::getById(pkgId);
                else
                    pkg = item->owner;

                PackageItem* pi = pkg ? pkg->getItem(src) : nullptr;
                if (pi)
                    di = new DisplayListItem(pi, "");
                else
                    di = new DisplayListItem(nullptr, cxml->Name());
            }
            else
            {
                if (strcmp(cxml->Name(), "text") == 0 && cxml->BoolAttribute("input"))
                    di = new DisplayListItem(nullptr, "inputtext");
                else
                    di = new DisplayListItem(nullptr, cxml->Name());
            }

            di->desc = cxml;
            item->displayList->push_back(di);

            cxml = cxml->NextSiblingElement();
        }
    }
}

void UIPackage::translateComponent(PackageItem * item)
{
    if (_stringsSource.empty())
        return;

    auto it = _stringsSource.find(_id + item->id);
    if (it == _stringsSource.end())
        return;

    const ValueMap& strings = it->second;
    std::string ename, elementId, value;
    const char* p;
    int dcnt = item->displayList->size();
    for (int i = 0; i < dcnt; i++)
    {
        TXMLElement* cxml = item->displayList->at(i)->desc;
        ename = cxml->Name();
        elementId = (p = cxml->Attribute("id")) ? p : STD_STRING_EMPTY;
        if (p = cxml->Attribute("tooltips"))
        {
            auto it = strings.find(elementId + "-tips");
            if (it != strings.end())
                cxml->SetAttribute("tooltips", it->second.asString().c_str());
        }

        TXMLElement* dxml = cxml->FirstChildElement("gearText");
        if (dxml != nullptr)
        {
            {
                auto it = strings.find(elementId + "-texts");
                if (it != strings.end())
                    dxml->SetAttribute("values", it->second.asString().c_str());
            }

            {
                auto it = strings.find(elementId + "-texts_def");
                if (it != strings.end())
                    dxml->SetAttribute("default", it->second.asString().c_str());
            }
        }

        if (ename == "text" || ename == "richtext")
        {
            {
                auto it = strings.find(elementId);
                if (it != strings.end())
                    cxml->SetAttribute("text", it->second.asString().c_str());
            }

            {
                auto it = strings.find(elementId + "-prompt");
                if (it != strings.end())
                    cxml->SetAttribute("prompt", it->second.asString().c_str());
            }
        }
        else if (ename == "list")
        {
            TXMLElement* exml = cxml->FirstChildElement("item");
            int j = 0;
            while (exml)
            {
                auto it = strings.find(elementId + "-" + Value(j).asString());
                if (it != strings.end())
                    exml->SetAttribute("title", it->second.asString().c_str());

                exml = exml->NextSiblingElement("item");
                j++;
            }
        }
        else if (ename == "component")
        {
            dxml = cxml->FirstChildElement("Button");
            if (dxml != nullptr)
            {
                {
                    auto it = strings.find(elementId);
                    if (it != strings.end())
                        dxml->SetAttribute("title", it->second.asString().c_str());
                }

                {
                    auto it = strings.find(elementId + "-0");
                    if (it != strings.end())
                        dxml->SetAttribute("selectedTitle", it->second.asString().c_str());
                }

                continue;
            }

            dxml = cxml->FirstChildElement("Label");
            if (dxml != nullptr)
            {
                {
                    auto it = strings.find(elementId);
                    if (it != strings.end())
                        dxml->SetAttribute("title", it->second.asString().c_str());
                }

                {
                    auto it = strings.find(elementId + "-prompt");
                    if (it != strings.end())
                        dxml->SetAttribute("prompt", it->second.asString().c_str());
                }

                continue;
            }

            dxml = cxml->FirstChildElement("ComboBox");
            if (dxml != nullptr)
            {
                {
                    auto it = strings.find(elementId);
                    if (it != strings.end())
                        dxml->SetAttribute("title", it->second.asString().c_str());
                }

                TXMLElement* exml = dxml->FirstChildElement("item");
                int j = 0;
                while (exml)
                {
                    auto it = strings.find(elementId + "-" + Value(j).asString());
                    if (it != strings.end())
                        exml->SetAttribute("title", it->second.asString().c_str());

                    exml = exml->NextSiblingElement("item");
                    j++;
                }

                continue;
            }
        }
    }
}

GObject * UIPackage::createObject(const std::string & resName)
{
    PackageItem* pi = getItemByName(resName);
    CCASSERT(pi, ("FairyGUI: resource not found - " + resName + " in " + _name).c_str());

    return createObject(pi);
}

GObject * UIPackage::createObject(PackageItem * item)
{
    loadItem(item);

    GObject* g = UIObjectFactory::newObject(item);
    if (g == nullptr)
        return nullptr;

    _constructing++;
    g->constructFromResource();
    _constructing--;
    return g;
}

NS_FGUI_END
