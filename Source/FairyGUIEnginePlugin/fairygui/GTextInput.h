#ifndef __GTEXTINPUT_H__
#define __GTEXTINPUT_H__

#include "FGUIMacros.h"
#include "GTextField.h"
#include "core/InputTextField.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP GTextInput : public GTextField
{
public:
    CREATE_FUNC(GTextInput);

    void setPrompt(const std::string& value);
    void setPassword(bool value);
    void setKeyboardType(int value);
    void setMaxLength(int value);
    void setRestrict(const std::string& value);

protected:
    GTextInput();
    virtual ~GTextInput();

protected:
    virtual void handleInit() override;
    virtual void getTextFieldText() override;
    virtual void setTextFieldText() override;
    virtual void setup_BeforeAdd(TXMLElement* xml) override;

private:
    InputTextField* _inputTextField;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GTextInput);
};

NS_FGUI_END

#endif
