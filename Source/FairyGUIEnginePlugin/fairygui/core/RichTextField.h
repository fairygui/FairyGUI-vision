#ifndef __RICHTEXTFIELD_H__
#define __RICHTEXTFIELD_H__

#include "FGUIMacros.h"
#include "TextField.h"

NS_FGUI_BEGIN

struct HtmlParseOptions;

class FGUI_IMPEXP RichTextField : public DisplayObject
{
public:
    CREATE_FUNC(RichTextField);

    TextField* getTextField() { return _textField; }

    virtual const std::string& getText() { return _textField->getText(); }
    virtual void setText(const std::string& value) { _textField->setText(value); }
    virtual void setHtmlText(const std::string& value) { _textField->setHtmlText(value); }

    TextFormat* getTextFormat() const { return _textField->_textFormat; }
    virtual void applyTextFormat() { _textField->applyTextFormat(); }

    virtual void ensureSizeCorrect() override { _textField->ensureSizeCorrect(); }
    virtual void update(float dt) override;

    HtmlParseOptions* getHtmlParseOptions() const { return _htmlParseOptions; }

protected:
    RichTextField();
    virtual ~RichTextField();

protected:
    virtual void onSizeChanged(bool widthChanged, bool heightChanged) override;

    TextField* _textField;
    HtmlParseOptions* _htmlParseOptions;

    friend class TextField;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(RichTextField);
};

NS_FGUI_END

#endif
