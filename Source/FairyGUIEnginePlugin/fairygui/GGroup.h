#ifndef __GGROUP_H__
#define __GGROUP_H__

#include "FGUIMacros.h"
#include "GObject.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP GGroup : public GObject
{
public:
    CREATE_FUNC(GGroup);

    GroupLayoutType getLayout() { return _layout; }
    void setLayout(GroupLayoutType value);

    int getColumnGap() { return _columnGap; }
    void setColumnGap(int value);

    int getLineGap() { return _lineGap; }
    void setLineGap(int value);

    void setBoundsChangedFlag(bool childSizeChanged = false);
    void moveChildren(float dx, float dy);
    void resizeChildren(float dw, float dh);

    int _updating;

protected:
    GGroup();
    virtual ~GGroup();

protected:
    virtual void setup_BeforeAdd(TXMLElement* xml) override;
    virtual void setup_AfterAdd(TXMLElement* xml) override;
    virtual void handleAlphaChanged() override;
    virtual void handleVisibleChanged() override;

private:
    void updateBounds();
    void handleLayout();
    void updatePercent();
    void ensureBoundsCorrect(float);

    GroupLayoutType _layout;
    int _lineGap;
    int _columnGap;
    bool _percentReady;
    bool _boundsChanged;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GGroup);
};

NS_FGUI_END

#endif
