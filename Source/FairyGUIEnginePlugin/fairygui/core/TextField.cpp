#include "TextField.h"
#include "UIConfig.h"
#include "UIPackage.h"
#include "RenderContext.h"
#include "InputTextField.h"
#include "HtmlHelper.h"
#include "FGUIManager.h"
#include "NativeFont.h"
#include "BitmapFont.h"
#include "utils/ToolSet.h"

#include <sstream>
#include <vector>
#include <locale>
#include <algorithm>

NS_FGUI_BEGIN

const int GUTTER_X = 2;
const int GUTTER_Y = 2;

class MyXmlVisitor : public tinyxml2::XMLVisitor
{
public:
    MyXmlVisitor(TextField* textField);

    virtual bool VisitEnter(const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute);
    virtual bool VisitExit(const tinyxml2::XMLElement& element);
    virtual bool Visit(const tinyxml2::XMLText& text);
    virtual bool Visit(const tinyxml2::XMLUnknown&) { return true; }

private:
    int attributeInt(std::unordered_map<std::string, const char*>& attrMap, const std::string& key, int defaultValue);

    void pushTextFormat();
    void popTextFormat();
    void addNewLine(bool check);
    void finishTextBlock();

    TextField *_textField;
    std::vector<TextFormat> _textFormatStack;
    std::vector<HtmlElement*> _linkStack;
    TextFormat _format;
    size_t _textFormatStackTop;
    int _skipText;
    bool _ignoreWhiteSpace;
    std::string _textBlock;
};

MyXmlVisitor::MyXmlVisitor(TextField* textField)
    : _textField(textField),
    _textFormatStackTop(0),
    _skipText(0),
    _ignoreWhiteSpace(false)
{
    _format = *textField->_textFormat;
}

void MyXmlVisitor::pushTextFormat()
{
    if (_textFormatStack.size() <= _textFormatStackTop)
        _textFormatStack.push_back(_format);
    else
        _textFormatStack[_textFormatStackTop] = _format;
    _textFormatStackTop++;
}

void MyXmlVisitor::popTextFormat()
{
    if (_textFormatStackTop > 0)
    {
        _format = _textFormatStack[_textFormatStackTop - 1];
        _textFormatStackTop--;
    }
}

void MyXmlVisitor::addNewLine(bool check)
{
    HtmlElement* lastElement = _textField->_htmlElements.empty() ? nullptr : _textField->_htmlElements.back();
    if (lastElement && lastElement->type == HtmlElement::Type::TEXT)
    {
        if (!check || lastElement->text.back() != '\n')
            lastElement->text += "\n";
        return;
    }

    HtmlElement* element = new HtmlElement(HtmlElement::Type::TEXT);
    element->format = _format;
    element->text = "\n";
    _textField->_htmlElements.push_back(element);
    if (!_linkStack.empty())
        element->link = _linkStack.back();
}

void MyXmlVisitor::finishTextBlock()
{
    if (!_textBlock.empty())
    {
        HtmlElement* element = new HtmlElement(HtmlElement::Type::TEXT);
        element->format = _format;
        element->text = _textBlock;
        _textBlock.clear();
        _textField->_htmlElements.push_back(element);
        if (!_linkStack.empty())
            element->link = _linkStack.back();
    }
}

bool MyXmlVisitor::VisitEnter(const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute)
{
    finishTextBlock();

    std::unordered_map<std::string, const char*> attrMap;
    for (const tinyxml2::XMLAttribute* attrib = firstAttribute; attrib; attrib = attrib->Next())
        attrMap[attrib->Name()] = attrib->Value();

    const char* elementName = element.Value();
    if (stricmp(elementName, "b") == 0)
    {
        pushTextFormat();
        _format.bold = true;
    }
    else if (stricmp(elementName, "i") == 0)
    {
        pushTextFormat();
        _format.italics = true;
    }
    else if (stricmp(elementName, "u") == 0)
    {
        pushTextFormat();
        _format.underline = true;
    }
    else if (stricmp(elementName, "font") == 0)
    {
        pushTextFormat();
        _format.size = (float)attributeInt(attrMap, "size", (int)_format.size);

        auto it = attrMap.find("color");
        if (it != attrMap.end())
        {
            _format.color = ToolSet::convertFromHtmlColor(it->second);
            _format._hasColor = true;
        }
    }
    else if (stricmp(elementName, "br") == 0)
    {
        addNewLine(false);
    }

    else if (stricmp(elementName, "img") == 0)
    {
        std::string src;

        int width = 0;
        int height = 0;

        auto it = attrMap.find("src");
        if (it != attrMap.end()) {
            src = it->second;
        }

        if (!src.empty()) {
            PackageItem* pi = UIPackage::getItemByURL(src);
            if (pi)
            {
                width = pi->width;
                height = pi->height;
            }
        }

        width = attributeInt(attrMap, "width", width);
        height = attributeInt(attrMap, "height", height);
        if (width == 0)
            width = 5;
        if (height == 0)
            height = 10;

        HtmlElement* element = new HtmlElement(HtmlElement::Type::IMAGE);
        element->width = width;
        element->height = height;
        element->text = src;
        _textField->_htmlElements.push_back(element);
        if (!_linkStack.empty())
            element->link = _linkStack.back();
    }

    else if (stricmp(elementName, "a") == 0)
    {
        pushTextFormat();

        std::string href;
        auto it = attrMap.find("href");
        if (it != attrMap.end())
            href = it->second;

        HtmlElement* element = new HtmlElement(HtmlElement::Type::LINK);
        element->text = href;
        _textField->_htmlElements.push_back(element);
        _linkStack.push_back(element);

        if (_textField->_richTextField && _textField->_richTextField->getHtmlParseOptions()->linkUnderline)
            _format.underline = true;
        if (_textField->_richTextField && !_format._hasColor)
            _format.color = _textField->_richTextField->getHtmlParseOptions()->linkColor;
    }
    else if (stricmp(elementName, "p") == 0)
    {
        addNewLine(true);
    }
    else if (stricmp(elementName, "html") == 0 || stricmp(elementName, "body") == 0)
        _ignoreWhiteSpace = true;


    return true;
}

bool MyXmlVisitor::VisitExit(const tinyxml2::XMLElement& element)
{
    finishTextBlock();

    const char* elementName = element.Value();
    if (stricmp(elementName, "b") == 0 || stricmp(elementName, "i") == 0 || stricmp(elementName, "u") == 0 || stricmp(elementName, "font") == 0)
        popTextFormat();
    else if (stricmp(elementName, "a") == 0)
    {
        popTextFormat();

        if (!_linkStack.empty())
            _linkStack.pop_back();
    }

    return true;
}

static bool isWhitespace(char c) {
    return std::isspace(c, std::locale());
}

static void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if_not(s.begin(),
        s.end(),
        isWhitespace));
}

static void rtrim(std::string& s) {
    s.erase(std::find_if_not(s.rbegin(),
        s.rend(),
        isWhitespace).base(),
        s.end());
}

bool MyXmlVisitor::Visit(const tinyxml2::XMLText& text)
{
    if (_skipText != 0)
        return true;

    if (_ignoreWhiteSpace)
    {
        std::string s(text.Value());
        ltrim(s);
        rtrim(s);
        _textBlock += s;
    }
    else
        _textBlock += text.Value();

    return true;
}

int MyXmlVisitor::attributeInt(std::unordered_map<std::string, const char*>& attrMap, const std::string& key, int defaultValue)
{
    auto it = attrMap.find(key);
    if (it != attrMap.end()) {
        int len = strlen(it->second);
        if (len != 0 && it->second[len - 1] == '%')
        {
            char* cc = new char[len];
            memcpy(cc, it->second, len);
            cc[len - 1] = '\0';
            int ret = (int)ceil(atoi(cc) / 100.0f*defaultValue);
            delete cc;
            return ret;
        }
        else
            return atoi(it->second);
    }
    else
        return defaultValue;
}

TextField::TextField() :
    _richTextField(nullptr),
    _font(nullptr),
    _input(false),
    _autoSize(TextAutoSize::NONE),
    _wordWrap(true),
    _singleLine(false),
    _html(false),
    _stroke(0),
    _strokeColor(V_RGBA_BLACK),
    _shadowOffset(0, 0),
    _textBounds(0, 0),
    _minHeight(0),
    _textChanged(false),
    _yOffset(0),
    _fontSizeScale(1),
    _charPositions(nullptr),
    _updatingSize(false)
{
    _touchDisabled = true;
    _textFormat = new TextFormat();
    _graphics = new NGraphics();
}

TextField::~TextField()
{
    delete _textFormat;

    for (auto &it : _htmlElements)
        delete it;
    for (auto &it : _renderElements)
        delete it;
    for (auto &it : _lines)
        delete it;
    CC_SAFE_DELETE(_charPositions);
}

void TextField::setInput()
{
    _input = true;
    _charPositions = new std::vector<CharPosition>();
    _textChanged = true;
}

void TextField::setText(const std::string & value)
{
    _text = value;
    _html = false;
    _textChanged = true;
}

void TextField::setHtmlText(const std::string& value)
{
    _text = value;
    _html = true;
    _textChanged = true;
}

void TextField::applyTextFormat()
{
    resolveFont();
    if (!_text.empty())
        _textChanged = true;
}

void TextField::setAutoSize(TextAutoSize value)
{
    if (_autoSize != value)
    {
        _autoSize = value;
        if (!_text.empty())
            _textChanged = true;
    }
}

void TextField::setSingleLine(bool value)
{
    if (_singleLine != value)
    {
        _singleLine = value;
        if (!_text.empty())
            _textChanged = true;
    }
}

void TextField::setWordWrap(bool value)
{
    if (_wordWrap != value)
    {
        _wordWrap = value;
        if (!_text.empty())
            _textChanged = true;
    }
}

void TextField::resolveFont()
{
    std::string fontName = _textFormat->font;
    if (fontName.empty())
        fontName = UIConfig::defaultFont;
    if (_font == nullptr || _font->getName() != fontName)
        _font = FGUIManager::GlobalManager().getFontByName(fontName);
    _graphics->setTexture(_font->getTexture());
}

void TextField::update(float dt)
{
    if (_richTextField == nullptr)
        rebuild();

    DisplayObject::update(dt);
}

const hkvVec2 & TextField::getTextSize()
{
    if (_textChanged)
        buildLines();

    return _textBounds;
}

bool TextField::rebuild()
{
    if (_font == nullptr)
        resolveFont();

    if (_textChanged)
        buildLines();

    if (_requireUpdateMesh)
    {
        buildMesh();
        return true;
    }
    else
        return false;
}

void TextField::ensureSizeCorrect()
{
    if (_textChanged && _autoSize != TextAutoSize::NONE)
        buildLines();
}

void TextField::onSizeChanged(bool widthChanged, bool heightChanged)
{
    if (!_updatingSize)
    {
        _minHeight = _contentRect.GetSizeY();

        if (_wordWrap && widthChanged)
            _textChanged = true;
        else if (_autoSize != TextAutoSize::NONE)
            _requireUpdateMesh = true;

        if (_textFormat->verticalAlign != VertAlignType::TOP)
            applyVerticalAlign();
    }

    DisplayObject::onSizeChanged(widthChanged, heightChanged);
}

void TextField::buildLines()
{
    _textChanged = false;
    _requireUpdateMesh = true;

    cleanup();

    if (_html && _text.length() > 0)
    {
        TXMLDocument xmlDoc;
        xmlDoc.Parse(("<xml>" + _text + "</xml>").c_str());
        MyXmlVisitor visitor(this);
        xmlDoc.Accept(&visitor);
    }

    if (_text.length() == 0 || _html && _htmlElements.size() == 0 || _font == nullptr)
    {
        LineInfo* emptyLine = new LineInfo();
        emptyLine->width = emptyLine->height = 0;
        emptyLine->y = emptyLine->y2 = GUTTER_Y;
        _lines.push_back(emptyLine);

        _textBounds.set(0, 0);
        _fontSizeScale = 1;

        buildLinesFinal();

        return;
    }

    int lineSpacing = _textFormat->lineSpacing - 1;
    float rectWidth = _contentRect.GetSizeX() - GUTTER_X * 2;
    float glyphWidth = 0, glyphHeight = 0;
    int charIndex = 0;

    TextFormat* format = _textFormat;
    bool wrap;
    if (_input)
        wrap = !_singleLine;
    else
        wrap = _wordWrap && !_singleLine;
    _fontSizeScale = 1;

    int elementCount = _htmlElements.size();
    int elementIndex = 0;
    HtmlElement* element = nullptr;
    if (elementCount > 0)
        element = _htmlElements[0];

    int lineIndex = 0;
    LineInfo* line = new LineInfo();
    _lines.push_back(line);
    line->y = line->y2 = GUTTER_Y;

    float lastLineHeight = 0;
    auto startNewLine = [&line, &lastLineHeight, &lineSpacing, &lineIndex, &format, this]()
    {
        if (line->width > _textBounds.x)
            _textBounds.x = line->width;

        if (line->textHeight == 0)
        {
            if (line->height == 0)
            {
                if (lastLineHeight == 0)
                    line->height = format->size;
                else
                    line->height = lastLineHeight;
            }
            line->textHeight = line->height;
        }
        lastLineHeight = line->height;

        LineInfo* newLine = new LineInfo();
        newLine->y = newLine->y2 = line->y + (line->height + lineSpacing);
        _lines.push_back(newLine);
        line = newLine;
        lineIndex++;
    };

    std::string textBlock = _html ? "" : _text;
    while (true)
    {
        if (element != nullptr)
        {
            if (element->type == HtmlElement::Type::TEXT)
            {
                format = &element->format;
                textBlock = element->text;

                TextRenderElement* re = new TextRenderElement();
                re->lineIndex = lineIndex;
                re->element = element;
                _renderElements.push_back(re);
            }
            else
            {
                IHtmlObject* htmlObject = nullptr;
                if (_richTextField != nullptr)
                {
                    element->space = (int)(rectWidth - line->width - 4);
                    element->createObject(_richTextField);
                    htmlObject = element->htmlObject;
                }
                if (htmlObject != nullptr)
                {
                    glyphWidth = (int)htmlObject->getWidth();
                    glyphHeight = (int)htmlObject->getHeight();
                }
                else
                    glyphWidth = 0;

                if (glyphWidth > 0)
                {
                    glyphWidth += 3;

                    if (line->width != 0)
                        line->width += format->letterSpacing;
                    line->width += glyphWidth;

                    if (wrap && line->width > rectWidth && line->width > glyphWidth)
                    {
                        line->width -= (glyphWidth + format->letterSpacing);
                        startNewLine();
                        line->width = glyphWidth;
                        line->height = glyphHeight;
                    }
                    else
                    {
                        if (glyphHeight > line->height)
                            line->height = glyphHeight;
                    }
                }

                TextRenderElement* re = new TextRenderElement();
                re->lineIndex = lineIndex;
                re->element = element;
                _renderElements.push_back(re);

                elementIndex++;
                if (elementIndex >= elementCount)
                    break;

                element = _htmlElements[elementIndex];
                continue;
            }
        }

        float measureWidth = rectWidth - line->width - format->letterSpacing;
        //from VTextState
        // Wrap text into individual lines
        const char *szCurrentLine = textBlock.c_str();
        while (*szCurrentLine)
        {
            // byte offsets
            int iByteOffsetAtWrapPosition;
            int iByteOffsetAfterWrapPosition;

            // search for next newline
            const char *pNextNewLine = strchr(szCurrentLine, '\n');

            // compute automatic wrap character index
            int iCharCount;
            if (wrap)
                iCharCount = _font->getCharacterIndexAtPos(szCurrentLine, *format, measureWidth, -1);
            else
                iCharCount = strlen(szCurrentLine);
            if (iCharCount == 0)
            {
                if (line->width > 0)
                {
                    measureWidth = _contentRect.GetSizeX();
                    startNewLine();
                    continue;
                }
                else
                    iCharCount = 1;
            }
            int iWrapOffset = VString::GetUTF8CharacterOffset(szCurrentLine, iCharCount);

            if (pNextNewLine != NULL && (pNextNewLine - szCurrentLine) <= iWrapOffset)
            {
                // newline occurs before automatic text wrap
                iByteOffsetAtWrapPosition = static_cast<int>(pNextNewLine - szCurrentLine);
                iByteOffsetAfterWrapPosition = iByteOffsetAtWrapPosition + 1;
            }
            else if (strlen(szCurrentLine) <= static_cast<size_t>(iWrapOffset))
            {
                // End of text occurs before automatic text wrap
                iByteOffsetAtWrapPosition = static_cast<int>(strlen(szCurrentLine));
                iByteOffsetAfterWrapPosition = iByteOffsetAtWrapPosition;
            }
            else
            {
                // automatic text wrap
                iByteOffsetAtWrapPosition = iWrapOffset;

                // Go backwards and try to find white space
                while (iByteOffsetAtWrapPosition > 0 && !hkvStringUtils::IsWhiteSpace(szCurrentLine[iByteOffsetAtWrapPosition]))
                {
                    iByteOffsetAtWrapPosition--;
                }

                // no whitespace found? then wrap inside word
                if (iByteOffsetAtWrapPosition == 0)
                {
                    iByteOffsetAtWrapPosition = iWrapOffset;
                }
                else
                {
                    // Find end of next word
                    int iEndOfWord = iByteOffsetAtWrapPosition + 1;
                    while (szCurrentLine[iEndOfWord] && !hkvStringUtils::IsWhiteSpace(szCurrentLine[iEndOfWord]))
                    {
                        iEndOfWord++;
                    }

                    // If the word does not fit into a line by itself, it will be wrapped anyway, so wrap it early to avoid ragged looking line endings
                    VRectanglef nextWordSize;
                    _font->getTextDimension(szCurrentLine + iByteOffsetAtWrapPosition, *format, nextWordSize, iEndOfWord - iByteOffsetAtWrapPosition);
                    if (nextWordSize.GetSizeX() > measureWidth)
                    {
                        iByteOffsetAtWrapPosition = iWrapOffset;
                    }
                }
                iByteOffsetAfterWrapPosition = iByteOffsetAtWrapPosition;
            }

            // put together line
            hkvStringView sView(szCurrentLine, szCurrentLine + iByteOffsetAtWrapPosition);
            hkvStringBuilder sLine = sView;

            VRectanglef rect(0, 0, 0, 0);
            _font->getTextDimension(szCurrentLine, *format, rect, iByteOffsetAtWrapPosition);
            float lineHeight = MAX(_font->getLineHeight(*format), rect.m_vMax.y);
            if (lineHeight > line->textHeight)
                line->textHeight = lineHeight;

            if (lineHeight > line->height)
                line->height = lineHeight;

            if (line->width != 0)
                line->width += format->letterSpacing;
            line->width += (wrap ? MIN(rectWidth - line->width, rect.m_vMax.x) : rect.m_vMax.x);

            TextRenderElement* re = new TextRenderElement();
            re->element = nullptr;
            re->charIndex = charIndex;
            re->lineIndex = lineIndex;
            re->size.set(rect.m_vMax.x, _font->getLineHeight(*format));
            re->text = std::string(szCurrentLine, iByteOffsetAtWrapPosition);
            re->charCount = iByteOffsetAfterWrapPosition;
            re->format = format;
            _renderElements.push_back(re);

            charIndex += re->charCount;

            measureWidth = _contentRect.GetSizeX();
            szCurrentLine = &szCurrentLine[iByteOffsetAfterWrapPosition];

            if (*szCurrentLine || pNextNewLine)
                startNewLine();
        }

        elementIndex++;
        if (elementIndex >= elementCount)
            break;

        element = _htmlElements[elementIndex];
    }

    line = _lines[_lines.size() - 1];

    if (line->width > _textBounds.x)
        _textBounds.x = line->width;
    if (_textBounds.x > 0)
        _textBounds.x += GUTTER_X * 2;

    if (_input && line->height == 0)
        line->height = _font->getLineHeight(*format);
    _textBounds.y = line->y + line->height + GUTTER_Y;

    _textBounds.x = hkvMath::ceil(_textBounds.x);
    _textBounds.y = hkvMath::ceil(_textBounds.y);
    if (_autoSize == TextAutoSize::SHRINK && _textBounds.x > rectWidth)
    {
        _fontSizeScale = rectWidth / _textBounds.x;
        _textBounds.x = rectWidth;
        _textBounds.y = hkvMath::ceil(_textBounds.y * _fontSizeScale);

        //调整各行的大小
        int lineCount = _lines.size();
        for (int i = 0; i < lineCount; ++i)
        {
            line = _lines[i];
            line->y *= _fontSizeScale;
            line->y2 *= _fontSizeScale;
            line->height *= _fontSizeScale;
            line->width *= _fontSizeScale;
            line->textHeight *= _fontSizeScale;
        }
    }
    else
        _fontSizeScale = 1;

    buildLinesFinal();
}

void TextField::buildLinesFinal()
{
    if (!_input && _autoSize == TextAutoSize::BOTH)
    {
        _updatingSize = true;
        if (_richTextField != nullptr)
            _richTextField->setSize(_textBounds.x, _textBounds.y);
        else
            setSize(_textBounds.x, _textBounds.y);
        _updatingSize = false;
    }
    else if (_autoSize == TextAutoSize::HEIGHT)
    {
        _updatingSize = true;
        float h = _textBounds.y;
        if (_input && h < _minHeight)
            h = _minHeight;
        if (_richTextField != nullptr)
            _richTextField->setHeight(h);
        else
            setHeight(h);
        _updatingSize = false;
    }

    _yOffset = 0;
    applyVerticalAlign();
}

void TextField::buildMesh()
{
    _requireUpdateMesh = false;
    _graphics->clearMesh();

    if (_textBounds.x == 0 && _lines.size() == 1)
    {
        if (_charPositions != nullptr)
        {
            _charPositions->clear();
            _charPositions->emplace_back();
        }

        if (_input)
            dynamic_cast<InputTextField*>(_richTextField)->onPostBuilt();

        return;
    }

    float rectWidth = _contentRect.GetSizeX() - GUTTER_X * 2;
    TextFormat* format = _textFormat;

    HtmlElement* currentLink = nullptr;
    float linkStartX = 0;
    int linkStartLine = 0;

    float charX = 0;
    float xIndent;
    int yIndent = 0;
    bool clipped = !_input && _autoSize == TextAutoSize::NONE;
    bool lineClipped = false;
    AlignType lineAlign;
    float lastGlyphHeight = 0;

    int TextRenderElementCount = _renderElements.size();
    int lastLineIndex = -1;
    LineInfo* line = nullptr;

    if (_charPositions != nullptr)
        _charPositions->clear();

    for (int i = 0; i < TextRenderElementCount; i++)
    {
        TextRenderElement* re = _renderElements[i];
        HtmlElement* element = re->element;

        if (re->lineIndex != lastLineIndex)
        {
            line = _lines[re->lineIndex];
            lastLineIndex = re->lineIndex;

            lineClipped = clipped && re->lineIndex != 0 && line->y + line->height > _contentRect.GetSizeX(); //超出区域，剪裁
            lineAlign = format->align;
            if (element != nullptr)
                lineAlign = element->format.align;
            else
                lineAlign = format->align;
            if (lineAlign == AlignType::CENTER)
                xIndent = hkvMath::floor((rectWidth - line->width) *0.5f);
            else
            {
                if (lineAlign == AlignType::RIGHT)
                    xIndent = rectWidth - line->width;
                else
                    xIndent = 0;
            }
            if (_input && xIndent < 0)
                xIndent = 0;
            charX = GUTTER_X + xIndent;
        }

        if (element != nullptr)
        {
            if (element->type == HtmlElement::Type::TEXT)
            {
                format = &element->format;
            }
            else if (element->type == HtmlElement::Type::IMAGE)
            {
                IHtmlObject* htmlObj = element->htmlObject;
                if (htmlObj != nullptr)
                {
                    element->position.set(charX + 1, line->y + (int)((line->height - htmlObj->getHeight()) / 2));
                    htmlObj->setPosition(element->position.x, element->position.y);
                    if (lineClipped || clipped && (element->position.x < GUTTER_X || element->position.x + htmlObj->getWidth() > _contentRect.GetSizeX() - GUTTER_X))
                        element->status |= 1;
                    else
                        element->status &= 254;
                    charX += htmlObj->getWidth() + format->letterSpacing + 2;
                }
            }

            if (element->link != currentLink)
            {
                if (currentLink && currentLink->htmlObject)
                {
                    ((HtmlLink*)currentLink->htmlObject)->setArea(linkStartLine, linkStartX, re->lineIndex, charX);
                    currentLink = nullptr;
                }
                currentLink = element->link;
                if (currentLink)
                {
                    if (currentLink && currentLink->htmlObject)
                    {
                        element->position = hkvVec2::ZeroVector();
                        currentLink->htmlObject->setPosition(0, 0);
                        linkStartX = charX;
                        linkStartLine = re->lineIndex;
                    }
                }
            }
        }
        else
        {
            if (!lineClipped)
            {
                yIndent = (int)((line->height + line->textHeight) / 2 - re->size.y);
                re->pos.set(charX, line->y + yIndent);
                _font->prepareGraphics(*_graphics, *re);

                if (_charPositions != nullptr)
                {
                    int len = re->charCount;
                    for (int si = 0; si < len; si++)
                        _charPositions->push_back(CharPosition(re->charIndex + si, re->lineIndex));
                }
            }

            charX += (int)hkvMath::ceil(re->size.x) + format->letterSpacing;
        }
    }

    if (currentLink != nullptr && currentLink->htmlObject)
        ((HtmlLink*)currentLink->htmlObject)->setArea(linkStartLine, linkStartX, _lines.size() - 1, charX);

    if (_charPositions != nullptr)
        _charPositions->push_back(CharPosition(_charPositions->size(), _lines.size() - 1));

    int count = _htmlElements.size();
    for (int i = 0; i < count; i++)
    {
        HtmlElement* element = _htmlElements[i];
        if (element->htmlObject != nullptr)
        {
            if ((element->status & 3) == 0) //not (hidden and clipped)
            {
                if ((element->status & 4) == 0) //not added
                {
                    element->status |= 4;
                    element->htmlObject->add();
                }
            }
            else
            {
                if ((element->status & 4) != 0) //added
                {
                    element->status &= 251;
                    element->htmlObject->remove();
                }
            }
        }
    }

    _graphics->drawText(dynamic_cast<NativeFont*>((BaseFont*)_font), &_renderElements);

    if (_input)
        dynamic_cast<InputTextField*>(_richTextField)->onPostBuilt();
}

void TextField::applyVerticalAlign()
{
    int oldOffset = _yOffset;
    if (_autoSize == TextAutoSize::BOTH || _autoSize == TextAutoSize::HEIGHT
        || _textFormat->verticalAlign == VertAlignType::TOP)
        _yOffset = 0;
    else
    {
        float dh;
        if (_textBounds.y == 0)
            dh = _contentRect.GetSizeY() - _textFormat->size;
        else
            dh = _contentRect.GetSizeY() - _textBounds.y;
        if (dh < 0)
            dh = 0;
        if (_textFormat->verticalAlign == VertAlignType::MIDDLE)
            _yOffset = (int)(dh / 2);
        else
            _yOffset = (int)dh;
    }

    if (oldOffset != _yOffset)
    {
        int cnt = _lines.size();
        for (int i = 0; i < cnt; i++)
            _lines[i]->y = _lines[i]->y2 + _yOffset;
        _requireUpdateMesh = true;
    }
}

void TextField::getLinesShape(int startLine, float startCharX, int endLine, float endCharX, bool clipped, std::vector<VRectanglef>& resultRects)
{
    LineInfo* line1 = _lines[startLine];
    LineInfo* line2 = _lines[endLine];
    if (startLine == endLine)
    {
        VRectanglef r(startCharX, line1->y, endCharX, line1->y + line1->height);
        if (clipped)
            resultRects.push_back(ToolSet::intersection(r, _contentRect));
        else
            resultRects.push_back(r);
    }
    else if (startLine == endLine - 1)
    {
        VRectanglef r(startCharX, line1->y, GUTTER_X + line1->width, line1->y + line1->height);
        if (clipped)
            resultRects.push_back(ToolSet::intersection(r, _contentRect));
        else
            resultRects.push_back(r);
        r.Set(GUTTER_X, line1->y + line1->height, endCharX, line2->y + line2->height);
        if (clipped)
            resultRects.push_back(ToolSet::intersection(r, _contentRect));
        else
            resultRects.push_back(r);
    }
    else
    {
        VRectanglef r(startCharX, line1->y, GUTTER_X + line1->width, line1->y + line1->height);
        if (clipped)
            resultRects.push_back(ToolSet::intersection(r, _contentRect));
        else
            resultRects.push_back(r);
        for (int i = startLine + 1; i < endLine; i++)
        {
            LineInfo* line = _lines[i];
            r.Set(GUTTER_X, r.m_vMax.y, GUTTER_X + line->width, line->y + line->height);
            if (clipped)
                resultRects.push_back(ToolSet::intersection(r, _contentRect));
            else
                resultRects.push_back(r);
        }
        r.Set(GUTTER_X, r.m_vMax.y, endCharX, line2->y + line2->height);
        if (clipped)
            resultRects.push_back(ToolSet::intersection(r, _contentRect));
        else
            resultRects.push_back(r);
    }
}

void TextField::cleanup()
{
    for (auto &it : _htmlElements)
        delete it;
    _htmlElements.clear();

    for (auto &it : _renderElements)
        delete it;
    _renderElements.clear();

    for (auto &it : _lines)
        delete it;
    _lines.clear();

    if (_charPositions != nullptr)
        _charPositions->clear();

    _textBounds.set(0, 0);
    _graphics->clearMesh();
}

NS_FGUI_END