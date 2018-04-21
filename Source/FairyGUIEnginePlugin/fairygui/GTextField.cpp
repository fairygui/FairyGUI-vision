#include "GTextField.h"
#include "utils/ToolSet.h"
#include "utils/UBBParser.h"

NS_FGUI_BEGIN

GTextField::GTextField() :
    _textField(nullptr),
    _ubbEnabled(false),
    _updatingSize(false)
{
}

GTextField::~GTextField()
{
}

void GTextField::handleInit()
{
    _textField = TextField::create();
    _textField->retain();

    _displayObject = _textField;
    initStyle();
}

void GTextField::initStyle()
{
    TextFormat* tf = _textField->getTextFormat();
    tf->size = 12;
    tf->color = V_RGBA_BLACK;
    tf->lineSpacing = 3;
    tf->letterSpacing = 0;
    _textField->applyTextFormat();

    _textField->setAutoSize(TextAutoSize::BOTH);
    _textField->setWordWrap(false);
}

const std::string & GTextField::getText() const
{
    const_cast<GTextField*>(this)->getTextFieldText();
    return _text;
}

void GTextField::setText(const std::string & value)
{
    _text = value;
    setTextFieldText();
    updateSize();
    updateGear(6);
}

void GTextField::getTextFieldText()
{
}

void GTextField::setTextFieldText()
{
    if (_ubbEnabled)
        _textField->setHtmlText(UBBParser::defaultParser.parse(_text.c_str()));
    else
        _textField->setText(_text);
}

void GTextField::applyTextFormat()
{
    _textField->applyTextFormat();
    if (!_underConstruct)
        updateSize();
}

void GTextField::setColor(const VColorRef & value)
{
    getTextFormat()->color = value;
    applyTextFormat();
    updateGear(4);
}

void GTextField::setFontSize(float value)
{
    getTextFormat()->size = value;
    applyTextFormat();
}

void GTextField::setOutlineColor(const VColorRef & value)
{
    getTextFormat()->outlineColor = value;
    applyTextFormat();
}

void GTextField::setAutoSize(TextAutoSize value)
{
    _textField->setAutoSize(value);
    if (value == TextAutoSize::BOTH)
    {
        _textField->setWordWrap(false);

        if (!_underConstruct)
            setSize(_textField->getTextSize().x, _textField->getTextSize().y);
    }
    else
    {
        _textField->setWordWrap(true);

        if (value == TextAutoSize::HEIGHT)
        {
            if (!_underConstruct)
                setHeight(_textField->getTextSize().y);
        }
        else
            _displayObject->setSize(_size.x, _size.y);
    }
}

void GTextField::setSingleLine(bool value)
{
    _textField->setSingleLine(value);
    if (!_underConstruct)
        updateSize();
}

void GTextField::updateSize()
{
    if (_updatingSize)
        return;

    _updatingSize = true;

    if (_textField->getAutoSize() == TextAutoSize::BOTH)
        setSize(_displayObject->getWidth(), _displayObject->getHeight());
    else if (_textField->getAutoSize() == TextAutoSize::HEIGHT)
        setHeight(_displayObject->getHeight());

    _updatingSize = false;
}

void GTextField::handleSizeChanged()
{
    if (_updatingSize)
        return;

    if (_underConstruct)
        _displayObject->setSize(_size.x, _size.y);
    else if (_textField->getAutoSize() != TextAutoSize::BOTH)
    {
        if (_textField->getAutoSize() == TextAutoSize::HEIGHT)
        {
            _displayObject->setWidth(_size.x); //先调整宽度，让文本重排
            if (!_text.empty()) //文本为空时，1是本来就不需要调整， 2是为了防止改掉文本为空时的默认高度，造成关联错误
                setSizeDirectly(_size.x, _displayObject->getHeight());
        }
        else
            _displayObject->setSize(_size.x, _size.y);
    }
}

void GTextField::setup_BeforeAdd(TXMLElement * xml)
{
    GObject::setup_BeforeAdd(xml);

    TextFormat* tf = getTextFormat();
    const char*p;
    p = xml->Attribute("font");
    if (p)
        tf->font = p;

    p = xml->Attribute("fontSize");
    if (p)
        tf->size = atoi(p);

    p = xml->Attribute("color");
    if (p)
        tf->color = ToolSet::convertFromHtmlColor(p);

    p = xml->Attribute("align");
    if (p)
        tf->align = ToolSet::parseAlign(p);

    p = xml->Attribute("vAlign");
    if (p)
        tf->verticalAlign = ToolSet::parseVerticalAlign(p);

    p = xml->Attribute("leading");
    if (p)
        tf->lineSpacing = atoi(p);

    p = xml->Attribute("letterSpacing");
    if (p)
        tf->letterSpacing = atoi(p);

    p = xml->Attribute("ubb");
    if (p)
        setUBBEnabled(strcmp(p, "true") == 0);

    p = xml->Attribute("autoSize");
    if (p)
        setAutoSize(ToolSet::parseTextAutoSize(p));

    p = xml->Attribute("underline");
    if (p)
        tf->underline = strcmp(p, "true") == 0;

    p = xml->Attribute("italic");
    if (p)
        tf->italics = strcmp(p, "true") == 0;

    p = xml->Attribute("bold");
    if (p)
        tf->bold = strcmp(p, "true") == 0;

    p = xml->Attribute("singleLine");
    if (p)
        setSingleLine(strcmp(p, "true") == 0);

    p = xml->Attribute("strokeColor");
    if (p)
    {
        tf->outlineColor = ToolSet::convertFromHtmlColor(p);
        p = xml->Attribute("strokeSize");
        tf->outlineSize = p ? atoi(p) : 1;
        tf->enableEffect(TextFormat::OUTLINE);
    }

    p = xml->Attribute("shadowColor");
    if (p)
    {
        tf->shadowColor = ToolSet::convertFromHtmlColor(p);

        p = xml->Attribute("shadowOffset");
        if (p)
            ToolSet::splitString(p, ',', tf->shadowOffset);
        tf->enableEffect(TextFormat::SHADOW);
    }

    _textField->applyTextFormat();
}

void GTextField::setup_AfterAdd(TXMLElement * xml)
{
    GObject::setup_AfterAdd(xml);

    const char* p;
    p = xml->Attribute("text");
    if (p && strlen(p) > 0)
        setText(p);
}

NS_FGUI_END
