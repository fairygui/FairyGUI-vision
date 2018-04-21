#include "RichTextField.h"
#include "HtmlHelper.h"

NS_FGUI_BEGIN

RichTextField::RichTextField()
{
    _textField = TextField::create();
    _textField->_richTextField = this;
    addChild(_textField);

    _htmlParseOptions = new HtmlParseOptions();

    setOpaque(true);
}

RichTextField::~RichTextField()
{
    CC_SAFE_DELETE(_htmlParseOptions);
}

void RichTextField::onSizeChanged(bool widthChanged, bool heightChanged)
{
    _textField->setSize(_contentRect.GetSizeX(), _contentRect.GetSizeY());

    DisplayObject::onSizeChanged(widthChanged, heightChanged);
}

void RichTextField::update(float dt)
{
    _textField->rebuild();

    DisplayObject::update(dt);
}

NS_FGUI_END