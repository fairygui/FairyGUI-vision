#ifndef __PACKAGEITEM_H__
#define __PACKAGEITEM_H__

#include "FGUIMacros.h"
#include "core/MovieClip.h"
#include "core/NTexture.h"
#include "core/BitmapFont.h"

NS_FGUI_BEGIN

class UIPackage;
class DisplayListItem;
class GComponent;
class NTexture;

class FGUI_IMPEXP PackageItem
{
public:
    PackageItem();
    virtual ~PackageItem();

    void load();

public:
    UIPackage* owner;

    PackageItemType type;
    std::string id;
    std::string name;
    int width;
    int height;
    std::string file;
    bool decoded;
    bool exported;

    //atlas
    NTexture* texture;

    //image
    VRectanglef* scale9Grid;
    bool scaleByTile;
    int tileGridIndice;

    //movieclip
    float interval;
    float repeatDelay;
    bool swing;
    hkvArray<MovieClip::Frame> frames;

    //component
    TXMLDocument* componentData;
    std::vector<DisplayListItem*>* displayList;
    std::function<GComponent*()> extensionCreator;

    //sound
    VFmodSoundObjectPtr sound;

    //font
    BitmapFont* bitmapFont;
};

class DisplayListItem
{
public:
    PackageItem* packageItem;
    std::string type;
    TXMLElement* desc;
    int listItemCount;

    DisplayListItem(PackageItem* pi, const std::string& type);
    virtual ~DisplayListItem();
};

NS_FGUI_END

#endif
