#ifndef __GSCROLLBAR_H__
#define __GSCROLLBAR_H__

#include "FGUIMacros.h"
#include "GComponent.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP GScrollBar : public GComponent
{
public:
    CREATE_FUNC(GScrollBar);

    void setScrollPane(ScrollPane* target, bool vertical);
    void setDisplayPerc(float value);
    void setScrollPerc(float value);
    float getMinSize();

protected:
    GScrollBar();
    virtual ~GScrollBar();

protected:
    virtual void constructFromXML(TXMLElement* xml) override;

private:
    void onTouchBegin(EventContext* context);
    void onGripTouchBegin(EventContext* context);
    void onGripTouchMove(EventContext* context);
    void onArrowButton1Click(EventContext* context);
    void onArrowButton2Click(EventContext* context);

    GObject* _grip;
    GObject* _arrowButton1;
    GObject* _arrowButton2;
    GObject* _bar;
    ScrollPane* _target;

    bool _vertical;
    float _scrollPerc;
    bool _fixedGripSize;

    hkvVec2 _dragOffset;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GScrollBar);
};

NS_FGUI_END

#endif
