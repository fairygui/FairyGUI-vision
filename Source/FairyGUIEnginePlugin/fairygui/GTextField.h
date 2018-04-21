#ifndef __GTEXTFIELD_H__
#define __GTEXTFIELD_H__

#include "FGUIMacros.h"
#include "GObject.h"
#include "core/TextField.h"
#include "gears/GearColor.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP GTextField : public GObject, public IColorGear
{
public:
    CREATE_FUNC(GTextField);

    const std::string& getText() const override;
    void setText(const std::string& value) override;

    TextAutoSize getAutoSize() const { return _textField->getAutoSize(); }
    virtual void setAutoSize(TextAutoSize value);

    bool isSingleLine() const { return _textField->isSingleLine(); }
    virtual void setSingleLine(bool value);

    TextFormat* getTextFormat() const { return _textField->getTextFormat(); }
    virtual void applyTextFormat();

    bool isUBBEnabled() const { return _ubbEnabled; }
    virtual void setUBBEnabled(bool value) { _ubbEnabled = value; }

    const hkvVec2& getTextSize() { return _textField->getTextSize(); }

    const VColorRef& getColor() const { return getTextFormat()->color; }
    void setColor(const VColorRef& value);

    float getFontSize() const { return getTextFormat()->size; }
    void setFontSize(float value);

    const VColorRef& getOutlineColor() const override { return getTextFormat()->outlineColor; }
    void setOutlineColor(const VColorRef& value) override;

protected:
    GTextField();
    virtual ~GTextField();

protected:
    virtual void handleInit() override;
    virtual void handleSizeChanged() override;
    virtual void setup_BeforeAdd(TXMLElement* xml) override;
    virtual void setup_AfterAdd(TXMLElement* xml) override;

    void initStyle();
    virtual void getTextFieldText();
    virtual void setTextFieldText();

    void updateSize();

    TextField* _textField;
    std::string _text;
    bool _ubbEnabled;
    bool _updatingSize;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GTextField);
};

NS_FGUI_END

#endif
