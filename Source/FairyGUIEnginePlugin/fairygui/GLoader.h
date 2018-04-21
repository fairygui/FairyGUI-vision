#ifndef __GLOADER_H__
#define __GLOADER_H__

#include "FGUIMacros.h"
#include "GObject.h"
#include "core/MovieClip.h"
#include "gears/GearColor.h"
#include "gears/GearAnimation.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP GLoader : public GObject, public IColorGear, public IAnimationGear
{
public:
    CREATE_FUNC(GLoader);

    const std::string& getURL() const { return _url; }
    void setURL(const std::string& value);

    virtual const std::string& getIcon() const override { return _url; }
    virtual void setIcon(const std::string& value) override { setURL(value); }

    AlignType getAlign() const { return _align; }
    void setAlign(AlignType value);

    VertAlignType getVerticalAlign() const { return _verticalAlign; }
    void setVerticalAlign(VertAlignType value);

    bool getAutoSize() const { return _autoSize; }
    void setAutoSize(bool value);

    LoaderFillType getFill() const { return _fill; }
    void setFill(LoaderFillType value);

    const VColorRef& getColor() const override { return _content->getColor(); }
    void setColor(const VColorRef& value) override;

    bool isPlaying() const override { return _content->isPlaying(); }
    void setPlaying(bool value) override;

    int getCurrentFrame() const override { return _content->getCurrentFrame(); }
    void setCurrentFrame(int value) override;

protected:
    GLoader();
    virtual ~GLoader();

protected:
    virtual void handleInit() override;
    virtual void handleSizeChanged() override;
    virtual void handleGrayedChanged() override;
    virtual void setup_BeforeAdd(TXMLElement* xml) override;

    virtual void loadExternal();
    void onExternalLoadSuccess(VTextureObject* texture);
    void onExternalLoadFailed();

private:
    void loadContent();
    void loadFromPackage();
    void clearContent();
    void updateLayout();
    void setErrorState();
    void clearErrorState();

    std::string _url;
    AlignType _align;
    VertAlignType _verticalAlign;
    bool _autoSize;
    LoaderFillType _fill;
    bool _updatingLayout;
    PackageItem* _contentItem;
    hkvVec2 _contentSize;
    hkvVec2 _contentSourceSize;
    int _contentStatus;

    MovieClip* _content;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GLoader);
};

NS_FGUI_END

#endif
