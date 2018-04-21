#ifndef __GLABEL_H__
#define __GLABEL_H__

#include "FGUIMacros.h"
#include "GComponent.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP GLabel : public GComponent
{
public:
    CREATE_FUNC(GLabel);

    const std::string& getTitle() { return _title; }
    void setTitle(const std::string& value);

    virtual const std::string& getText() const override { return _title; }
    virtual void setText(const std::string& value) override { setTitle(value); }

    virtual const std::string& getIcon() const override { return _icon; }
    virtual void setIcon(const std::string& value) override;

    const VColorRef& getTitleColor() const;
    void setTitleColor(const VColorRef& value);

    float getTitleFontSize() const;
    void setTitleFontSize(float value);

protected:
    GLabel();
    virtual ~GLabel();

protected:
    virtual void constructFromXML(TXMLElement* xml) override;
    virtual void setup_AfterAdd(TXMLElement* xml) override;

private:
    GObject* _titleObject;
    GObject* _iconObject;
    std::string _title;
    std::string _icon;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GLabel);
};

NS_FGUI_END

#endif
