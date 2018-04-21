#ifndef __GSLIDER_H__
#define __GSLIDER_H__

#include "FGUIMacros.h"
#include "GComponent.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP GSlider : public GComponent
{
public:
    CREATE_FUNC(GSlider);

    ProgressTitleType getTitleType() const { return _titleType; }
    void setTitleType(ProgressTitleType value);

    double getMax() const { return _max; }
    void setMax(double value);

    double getValue() const { return _value; }
    void setValue(double value);

    bool changeOnClick;
    bool canDrag;

protected:
    GSlider();
    virtual ~GSlider();

protected:
    virtual void handleSizeChanged() override;
    virtual void constructFromXML(TXMLElement* xml) override;
    virtual void setup_AfterAdd(TXMLElement* xml) override;

    void update();
    void updateWidthPercent(float percent);

private:
    void onTouchBegin(EventContext* context);
    void onGripTouchBegin(EventContext* context);
    void onGripTouchMove(EventContext* context);

    double _max;
    double _value;
    ProgressTitleType _titleType;
    bool _reverse;

    GObject* _titleObject;
    GObject* _barObjectH;
    GObject* _barObjectV;
    float _barMaxWidth;
    float _barMaxHeight;
    float _barMaxWidthDelta;
    float _barMaxHeightDelta;
    GObject* _gripObject;
    hkvVec2 _clickPos;
    float _clickPercent;
    float _barStartX;
    float _barStartY;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GSlider);
};

NS_FGUI_END

#endif
