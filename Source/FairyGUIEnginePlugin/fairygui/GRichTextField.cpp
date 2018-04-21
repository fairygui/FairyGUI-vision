#include "GRichTextField.h"
#include "utils/UBBParser.h"

NS_FGUI_BEGIN

GRichTextField::GRichTextField() :
    _richTextField(nullptr)
{
}

GRichTextField::~GRichTextField()
{
}

void GRichTextField::handleInit()
{
    _richTextField = RichTextField::create();
    _richTextField->retain();
    _textField = _richTextField->getTextField();

    _displayObject = _richTextField;

    initStyle();
}

void GRichTextField::getTextFieldText()
{
    _text = _richTextField->getText();
}

void GRichTextField::setTextFieldText()
{
    if (_ubbEnabled)
        _richTextField->setHtmlText(UBBParser::defaultParser.parse(_text.c_str()));
    else
        _richTextField->setHtmlText(_text);
}

NS_FGUI_END
