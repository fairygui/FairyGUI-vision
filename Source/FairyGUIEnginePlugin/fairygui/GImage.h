#ifndef __GIMAGE_H__
#define __GIMAGE_H__

#include "core/Image.h"
#include "FGUIMacros.h"
#include "GObject.h"
#include "gears/GearColor.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP GImage : public GObject, public IColorGear
{
public:
    CREATE_FUNC(GImage);

    FlipType getFlip() const { return _content->getFlip(); }
    void setFlip(FlipType value) { _content->setFlip(value); }

    const VColorRef& getColor() const override { return _content->getColor(); }
    void setColor(const VColorRef& value) override;

    virtual void constructFromResource() override;

protected:
    GImage();
    virtual ~GImage();

protected:
    virtual void handleInit() override;
    virtual void setup_BeforeAdd(TXMLElement* xml) override;

private:
    Image* _content;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GImage);
};

NS_FGUI_END

#endif
