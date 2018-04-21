#include "GTextInput.h"
#include "UIPackage.h"

NS_FGUI_BEGIN

GTextInput::GTextInput():_inputTextField(nullptr)
{
}

GTextInput::~GTextInput()
{
}

void GTextInput::handleInit()
{
    _inputTextField = InputTextField::create();
    _inputTextField->retain();
    _textField = _inputTextField->getTextField();

    _displayObject = _inputTextField;

    initStyle();
    _textField->setAutoSize(TextAutoSize::NONE);
    _textField->setWordWrap(false);
}

void GTextInput::setPrompt(const std::string & value)
{
    _inputTextField->setPromptText(value);
}

void GTextInput::setPassword(bool value)
{
    _inputTextField->setDisplayAsPassword(value);
}

void GTextInput::setKeyboardType(int value)
{
    //_inputTextField->setKeyboardType(value);
}

void GTextInput::setMaxLength(int value)
{
    _inputTextField->setMaxLength(value);
}

void GTextInput::setRestrict(const std::string & value)
{
}

void GTextInput::getTextFieldText()
{
    _text = _inputTextField->getText();
}

void GTextInput::setTextFieldText()
{
    _inputTextField->setText(_text);
}

void GTextInput::setup_BeforeAdd(TXMLElement * xml)
{
    GTextField::setup_BeforeAdd(xml);

    const char *p;

    p = xml->Attribute("prompt");
    if (p)
        setPrompt(p);

    if (xml->BoolAttribute("password"))
        setPassword(true);

    p = xml->Attribute("restrict");
    if (p)
        setRestrict(p);

    p = xml->Attribute("maxLength");
    if (p)
        setMaxLength(atoi(p));

    p = xml->Attribute("keyboardType");
    if (p)
        setKeyboardType(atoi(p));
}

NS_FGUI_END