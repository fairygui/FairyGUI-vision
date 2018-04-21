#ifndef __GMOVIECLIP_H__
#define __GMOVIECLIP_H__

#include "FGUIMacros.h"
#include "GObject.h"
#include "core/MovieClip.h"
#include "gears/GearColor.h"
#include "gears/GearAnimation.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP GMovieClip : public GObject, public IColorGear, public IAnimationGear
{
public:
    CREATE_FUNC(GMovieClip);

    bool isPlaying() const override { return _content->isPlaying(); }
    void setPlaying(bool value) override;

    int getCurrentFrame() const override { return _content->getCurrentFrame(); }
    void setCurrentFrame(int value) override;

    FlipType getFlip() const { return _content->getFlip(); }
    void setFlip(FlipType value) { _content->setFlip(value); }

    const VColorRef& getColor() const override { return _content->getColor(); }
    void setColor(const VColorRef& value) override;

    //from start to end(-1 means ending)£¬repeat times(0 means infinite loop)£¬when all is over, stopping at endAt(-1 means same value of end)
    void setPlaySettings(int start = 0, int end = -1, int times = 0, int endAt = -1, std::function<void()> completeCallback = nullptr);

    virtual void constructFromResource() override;

protected:
    GMovieClip();
    virtual ~GMovieClip();

protected:
    virtual void handleInit() override;
    virtual void setup_BeforeAdd(TXMLElement* xml) override;

private:
    MovieClip* _content;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GMovieClip);
};

NS_FGUI_END

#endif
