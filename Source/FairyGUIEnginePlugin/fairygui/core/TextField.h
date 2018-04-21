#ifndef __TEXTFIELD_H__
#define __TEXTFIELD_H__

#include "FGUIMacros.h"
#include "TextFormat.h"
#include "DisplayObject.h"
#include "BaseFont.h"

NS_FGUI_BEGIN

class RichTextField;
class HtmlElement;
class TextRenderElement;

class FGUI_IMPEXP TextField : public DisplayObject
{
public:
    CREATE_FUNC(TextField);

    const std::string& getText() { return _text; }
    void setText(const std::string& value);
    void setHtmlText(const std::string& value);

    TextFormat* getTextFormat() const { return _textFormat; }
    void applyTextFormat();

    TextAutoSize getAutoSize() const { return _autoSize; }
    void setAutoSize(TextAutoSize value);

    bool isSingleLine() const { return _singleLine; }
    void setSingleLine(bool value);

    bool isWordWrap() const { return _wordWrap; }
    void setWordWrap(bool value);

    const hkvVec2& getTextSize();

    bool rebuild();

    void getLinesShape(int startLine, float startCharX, int endLine, float endCharX,
        bool clipped, std::vector<VRectanglef>& resultRects);

    virtual void ensureSizeCorrect() override;
    virtual void onSizeChanged(bool widthChanged, bool heightChanged) override;
    virtual void update(float dt) override;

    struct LineInfo
    {
        LineInfo() : width(0), height(0), textHeight(0), y(0), y2(0) {}

        float width;
        float height;
        float textHeight;
        float y;
        float y2;
    };

    struct CharPosition
    {
        CharPosition() :charIndex(0), lineIndex(0) {}
        CharPosition(int c, int l) : charIndex(c), lineIndex(l) {}

        int charIndex;
        int lineIndex;
    };

protected:
    TextField();
    virtual ~TextField();

private:
    void resolveFont();
    void buildLines();
    void buildLinesFinal();
    void buildMesh();
    void applyVerticalAlign();
    void setInput();
    void cleanup();

    RichTextField* _richTextField;
    TextFormat* _textFormat;
    bool _input;
    std::string _text;
    TextAutoSize _autoSize;
    bool _wordWrap;
    bool _singleLine;
    bool _html;
    int _stroke;
    VColorRef _strokeColor;
    hkvVec2 _shadowOffset;

    BaseFont* _font;
    hkvVec2 _textBounds;
    float _minHeight;
    bool _textChanged;
    int _yOffset;
    float _fontSizeScale;
    bool _updatingSize;
    float _renderScale;

    std::vector<HtmlElement*> _htmlElements;
    std::vector<LineInfo*> _lines;
    std::vector<TextRenderElement*> _renderElements;
    std::vector<CharPosition>* _charPositions;

    friend class MyXmlVisitor;
    friend class RichTextField;
    friend class InputTextField;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(TextField);
};

class TextRenderElement
{
public:
    TextRenderElement() : lineIndex(0), charIndex(0), charCount(0), size(0, 0), pos(0, 0), format(nullptr), element(nullptr) { }

    int lineIndex;
    int charIndex;
    int charCount;
    std::string text;
    hkvVec2 size;
    hkvVec2 pos;
    TextFormat* format;
    HtmlElement* element;
};

NS_FGUI_END

#endif
