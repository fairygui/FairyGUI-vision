#include "InputTextField.h"
#include "UIPackage.h"
#include "IMEAdapter.h"
#include "FGUIManager.h"
#include "Shape.h"
#include "SelectionShape.h"
#include "UIConfig.h"
#include "HtmlHelper.h"
#include "HitTest.h"
#include "utils/UBBParser.h"

NS_FGUI_BEGIN

#ifdef SUPPORTS_SOFTKEYBOARD
#include <Vision/Runtime/Base/Input/VSoftKeyboardAdapter.hpp>

class SoftKeyboardHelper : public IVSoftKeyboardRecipient
{
public:
    SoftKeyboardHelper(InputTextField* owner) { _owner = owner; }

    virtual void EnterText(const char* pUTF8Text)
    {
        _owner->replaceSelection(pUTF8Text);
    }

    virtual bool HasText()
    {
        return !_owner->_text.empty();
    }

    virtual const char* GetUTF8Text() const
    {
        return _owner->_text.c_str();
    }

    virtual const char* GetDescription() const
    {
        return "";
    }

    virtual bool IsPassword() const
    {
        return _owner->_displayAsPassword;
    }

    virtual void OnSpecialKey(unsigned int uiKey)
    {
        _owner->handleSpecialKey((int)uiKey, false, false);
    }

    InputTextField* _owner;
};
#endif

const int GUTTER_X = 2;
const int GUTTER_Y = 2;

float InputTextField::_nextBlink = 0;

InputTextField::InputTextField() :
    _editable(true),
    _displayAsPassword(false),
    _maxLength(0),
    _editing(false),
    _caretPosition(0),
    _selectionStart(0),
    _composing(0)
{
    setTouchChildren(false);
    addListener(UIEventType::FocusIn, CALLBACK_1(InputTextField::onFocusIn, this));
    addListener(UIEventType::FocusOut, CALLBACK_1(InputTextField::onFocusOut, this), EventTag::None, INT_MAX);
    addListener(UIEventType::TouchBegin, CALLBACK_1(InputTextField::onTouchBegin, this), EventTag::None, INT_MAX);
    addListener(UIEventType::TouchMove, CALLBACK_1(InputTextField::onTouchMove, this), EventTag::None, INT_MAX);
    addListener(UIEventType::KeyDown, CALLBACK_1(InputTextField::onKeyDown, this), EventTag::None, INT_MAX);

#ifdef SUPPORTS_SOFTKEYBOARD
    _softkeyBoardHelper = new SoftKeyboardHelper(this);
#endif

    setHitArea(new RectHitTest());
    _textField->setInput();
}

InputTextField::~InputTextField()
{
#ifdef SUPPORTS_SOFTKEYBOARD
    CC_SAFE_DELETE(_softkeyBoardHelper);
#endif
}

void InputTextField::setText(const std::string & value)
{
    _text = value;
    clearSelection();
    updateText();
}

void InputTextField::applyTextFormat()
{
    RichTextField::applyTextFormat();
    if (_editing)
    {
        StageInst->getCaret()->setHeight(_textField->getTextFormat()->size);
        StageInst->getCaret()->drawRect(0, VColorRef(0, 0, 0, 0), _textField->getTextFormat()->color);
    }
}

void InputTextField::updateText()
{
    int composing = _composing;
    _composing = 0;

    if (!_editing && _text.empty() && !_promptText.empty())
        _textField->setHtmlText(_promptText);
    else if (_displayAsPassword)
        _textField->setText(encodePasswordText(_text));
    else if (!IMEAdapter::getCompositionString().empty())
    {
        _composing = IMEAdapter::getCompositionString().size();
        _textField->setText(_text.substr(0, _caretPosition) + IMEAdapter::getCompositionString() + _text.substr(_caretPosition + composing));
    }
    else
        _textField->setText(_text);
}

std::string InputTextField::encodePasswordText(const std::string & value)
{
    int textLen = value.size();
    hkvStringBuilder builder;
    int i = 0;
    while (i < textLen)
    {
        char c = value[i];
        if (c == '\n')
            builder.Append(c);
        else
            builder.Append("*");
        i++;
    }
    return std::string(builder.AsChar());
}

const TextField::CharPosition& InputTextField::getCharPosition(int caretIndex)
{
    if (caretIndex < 0)
        caretIndex = 0;
    else if (caretIndex >= (int)_textField->_charPositions->size())
        caretIndex = _textField->_charPositions->size() - 1;

    return _textField->_charPositions->at(caretIndex);
}

const TextField::CharPosition& InputTextField::getCharPosition(const hkvVec2 & location)
{
    if (_textField->_charPositions->size() <= 1)
        return _textField->_charPositions->at(0);

    float cx = location.x - _textField->getX();
    float cy = location.y - _textField->getY();

    int len = _textField->_lines.size();
    int lineIndex;
    for (lineIndex = 0; lineIndex < len; lineIndex++)
    {
        TextField::LineInfo* line = _textField->_lines[lineIndex];
        if (line->y + line->height > cy)
            break;
    }
    if (lineIndex == len)
        lineIndex = len - 1;

    for (auto &re : _textField->_renderElements)
    {
        if (re->lineIndex == lineIndex)
        {
            if (cx == 0)
                return _textField->_charPositions->at(re->charIndex);
            else if (cx >= re->pos.x + re->size.x - 2)
                return _textField->_charPositions->at(re->charIndex + re->text.size());
            else
            {
                int pos = _textField->_font->getCharacterIndexAtPos(re->text.c_str(), *re->format, cx, -1);
                return _textField->_charPositions->at(re->charIndex + pos);
            }
        }
    }

    return _textField->_charPositions->back();
}

hkvVec2 InputTextField::getCharLocation(const TextField::CharPosition & cp)
{
    TextField::LineInfo* line = _textField->_lines[cp.lineIndex];
    hkvVec2 pos(0, 0);
    if (line->width == 0 || _textField->_charPositions->size() == 0)
    {
        if (_textField->getTextFormat()->align == AlignType::CENTER)
            pos.x = hkvMath::floor(_contentRect.GetSizeX() * 0.5f);
        else
            pos.x = GUTTER_X;
    }
    else
    {
        TextField::CharPosition v = _textField->_charPositions->at(hkvMath::Min<int>(cp.charIndex, (int)(_textField->_charPositions->size() - 1)));
        for (auto &re : _textField->_renderElements)
        {
            if (re->lineIndex == v.lineIndex)
            {
                if (re->charIndex == v.charIndex)
                    pos.x = GUTTER_X;
                else
                {
                    VRectanglef rect;
                    _textField->_font->getTextDimension(re->text.c_str(), *re->format, rect, v.charIndex - re->charIndex);
                    pos.x = rect.m_vMax.x + 1;
                }
                break;
            }
        }
    }
    pos.x += _textField->getX();
    pos.y = _textField->getY() + line->y;
    return pos;
}

void InputTextField::adjustCaret(const TextField::CharPosition& cp, bool moveSelectionHeader)
{
    _caretPosition = cp.charIndex;
    if (moveSelectionHeader)
        _selectionStart = _caretPosition;

    updateCaret(false);
}

void InputTextField::updateCaret(bool forceUpdate)
{
    const TextField::CharPosition& cp = getCharPosition(_caretPosition + (_editing ? IMEAdapter::getCompositionString().size() : 0));

    hkvVec2 pos = getCharLocation(cp);
    TextField::LineInfo* line = _textField->_lines[cp.lineIndex];
    pos.y = line->y + _textField->getY();
    hkvVec2 newPos = pos;

    if (newPos.x < _textField->getTextFormat()->size)
        newPos.x += MIN(50, (int)(_contentRect.GetSizeX() / 2));
    else if (newPos.x > _contentRect.GetSizeX() - GUTTER_X - _textField->getTextFormat()->size)
        newPos.x -= MIN(50, (int)(_contentRect.GetSizeX() / 2));

    if (newPos.x < GUTTER_X)
        newPos.x = GUTTER_X;
    else if (newPos.x > _contentRect.GetSizeX() - GUTTER_X)
        newPos.x = MAX(GUTTER_X, _contentRect.GetSizeX() - GUTTER_X);

    if (newPos.y < GUTTER_Y)
        newPos.y = GUTTER_Y;
    else if (newPos.y + line->height >= _contentRect.GetSizeY() - GUTTER_Y)
        newPos.y = MAX(GUTTER_Y, _contentRect.GetSizeY() - line->height - GUTTER_Y);

    pos += moveContent(newPos - pos, forceUpdate);

    if (_editing)
    {
        if (line->height > 0) //将光标居中
            pos.y += (int)(line->height - _textField->getTextFormat()->size) / 2;

        StageInst->getCaret()->setPosition(pos.x, pos.y);

        hkvVec2 cursorPos = StageInst->getCaret()->localToGlobal(hkvVec2(0, StageInst->getCaret()->getHeight()));
        IMEAdapter::setCursorPos(cursorPos);

        _nextBlink = Vision::GetTimer()->GetTime() + 0.5f;
        StageInst->getCaret()->getGraphics()->setEnabled(true);

        updateSelection(cp);
    }
}

hkvVec2 InputTextField::moveContent(const hkvVec2& delta, bool forceUpdate)
{
    float ox = _textField->getX();
    float oy = _textField->getY();
    float nx = ox + delta.x;
    float ny = oy + delta.y;
    if (_contentRect.GetSizeX() - nx > _textField->_textBounds.x)
        nx = _contentRect.GetSizeX() - _textField->_textBounds.x;
    if (_contentRect.GetSizeY() - ny > _textField->_textBounds.y)
        ny = _contentRect.GetSizeY() - _textField->_textBounds.y;
    if (nx > 0)
        nx = 0;
    if (ny > 0)
        ny = 0;
    nx = hkvMath::floor(nx);
    ny = hkvMath::floor(ny);

    if (nx != ox || ny != oy || forceUpdate)
    {
        _textField->setPosition(nx, ny);

        int count = _textField->_htmlElements.size();
        for (int i = 0; i < count; i++)
        {
            HtmlElement* element = _textField->_htmlElements[i];
            if (element->htmlObject != nullptr)
                element->htmlObject->setPosition(element->position.x + nx, element->position.y + ny);
        }
    }

    return hkvVec2(nx - ox, ny - oy);
}

void InputTextField::clearSelection()
{
    if (_selectionStart != _caretPosition)
    {
        if (_editing)
            StageInst->getSelectionShape()->clear();
        _selectionStart = _caretPosition;
    }
}

std::string InputTextField::getSelection()
{
    if (_selectionStart == _caretPosition)
        return "";

    if (_selectionStart < _caretPosition)
        return _text.substr(_selectionStart, _caretPosition);
    else
        return _text.substr(_caretPosition, _selectionStart);
}

void InputTextField::updateSelection(const TextField::CharPosition& cp)
{
    if (_selectionStart == _caretPosition)
    {
        StageInst->getSelectionShape()->clear();
        return;
    }

    const TextField::CharPosition* pStart;
    const TextField::CharPosition* pEnd = &cp;
    if (_editing && !IMEAdapter::getCompositionString().empty())
    {
        if (_selectionStart < _caretPosition)
        {
            pEnd = &getCharPosition(_caretPosition);
            pStart = &getCharPosition(_selectionStart);
        }
        else
            pStart = &getCharPosition(_selectionStart + IMEAdapter::getCompositionString().size());
    }
    else
        pStart = &getCharPosition(_selectionStart);
    if (pStart->charIndex > pEnd->charIndex)
    {
        const TextField::CharPosition* tmp = pStart;
        pStart = pEnd;
        pEnd = tmp;
    }

    hkvVec2 v1 = getCharLocation(*pStart);
    hkvVec2 v2 = getCharLocation(*pEnd);

    std::vector<VRectanglef> rects;
    _textField->getLinesShape(pStart->lineIndex, v1.x - _textField->getX(), pEnd->lineIndex, v2.x - _textField->getX(), false, rects);
    StageInst->getSelectionShape()->setRects(rects);
    StageInst->getSelectionShape()->setPosition(_textField->getX(), _textField->getY());
}

void InputTextField::onPostBuilt()
{
    if (_editing)
    {
        setChildIndex(StageInst->getSelectionShape(), numChildren() - 1);
        setChildIndex(StageInst->getCaret(), numChildren() - 2);
    }

    int cnt = _textField->_charPositions->size();
    if (_caretPosition >= cnt)
        _caretPosition = cnt - 1;
    if (_selectionStart >= cnt)
        _selectionStart = cnt - 1;

    updateCaret(true);
}

void InputTextField::setPromptText(const std::string & value)
{
    _promptText = value;
    if (!_promptText.empty())
        _promptText = UBBParser::defaultParser.parse(_promptText.c_str());
    updateText();
}

void InputTextField::setDisplayAsPassword(bool value)
{
    if (_displayAsPassword != value)
    {
        _displayAsPassword = value;
        updateText();
    }
}

void InputTextField::setRestrict(const std::string & value)
{
}

void InputTextField::replaceSelection(const std::string & value)
{
    CCASSERT(_editable, "InputTextField is not editable.");

    if (!_editing)
        StageInst->setFocus(this);

    _textField->rebuild();

    int t0, t1;
    if (_selectionStart != _caretPosition)
    {
        if (_selectionStart < _caretPosition)
        {
            t0 = _selectionStart;
            t1 = _caretPosition;
            _caretPosition = _selectionStart;
        }
        else
        {
            t0 = _caretPosition;
            t1 = _selectionStart;
            _selectionStart = _caretPosition;
        }
    }
    else
    {
        if (value.empty())
            return;

        t0 = t1 = _caretPosition;
    }

    std::string newText = _text.substr(0, t0);
    if (!value.empty())
    {
        newText = newText + value;
        _caretPosition += value.length();
    }
    newText += _text.substr(t1 + _composing);

    if (_maxLength > 0 && _maxLength > (int)newText.length())
        newText = newText.substr(0, _maxLength);

    setText(newText);
    dispatchEvent(UIEventType::Changed);
}

void InputTextField::replaceText(const std::string & value)
{
    if (value == _text)
        return;

    std::string newText = value;

    if (_maxLength > 0 && _maxLength > (int)newText.length())
        newText = newText.substr(0, _maxLength);

    setText(newText);
    dispatchEvent(UIEventType::Changed);
}

void InputTextField::update(float dt)
{
    RichTextField::update(dt);

    if (_editing)
    {
        float curr = Vision::GetTimer()->GetTime();
        if (_nextBlink < curr)
        {
            _nextBlink = curr + 0.5f;
            StageInst->getCaret()->getGraphics()->blink();
        }
    }
}

void InputTextField::onSizeChanged(bool widthChanged, bool heightChanged)
{
    RichTextField::onSizeChanged(widthChanged, heightChanged);

    VRectanglef rect = _contentRect;
    rect.m_vMin.x += GUTTER_X;
    rect.m_vMin.y += GUTTER_Y;
    rect.m_vMax.x -= GUTTER_X; //高度不减GUTTER_X * 2，因为怕高度不小心截断文字

    setClipRect(rect);
    dynamic_cast<RectHitTest *>(getHitArea())->rect = _contentRect;
}

void InputTextField::onFocusIn(EventContext* context)
{
    if (!_editable)
        return;

    _editing = true;

    if (!_promptText.empty())
        updateText();

    float caretSize;
    //如果界面缩小过，光标很容易看不见，这里放大一下
    //if (UIConfig.inputCaretSize == 1 && GRoot.contentScaleFactor < 1)
    //    caretSize = (float)UIConfig.inputCaretSize / GRoot.contentScaleFactor;
    //else
    caretSize = UIConfig::inputCaretSize;
    StageInst->getCaret()->setSize(caretSize, _textField->getTextFormat()->size);
    StageInst->getCaret()->drawRect(0, VColorRef(0, 0, 0, 0), _textField->getTextFormat()->color);
    addChild(StageInst->getCaret());

    StageInst->getSelectionShape()->clear();
    addChild(StageInst->getSelectionShape());

    if (!_textField->rebuild())
    {
        const TextField::CharPosition& cp = getCharPosition(_caretPosition);
        adjustCaret(cp);
    }

#ifdef SUPPORTS_SOFTKEYBOARD
    VInputManager::GetSoftkeyboardAdapter().SetCurrentRecipient(_softkeyBoardHelper);
    VInputManager::GetSoftkeyboardAdapter().Show();
#else
    IMEAdapter::setCompositionMode(IMEAdapter::CompositionMode::On);
    _composing = 0;
#endif
}

void InputTextField::onFocusOut(EventContext* context)
{
    if (!_editing)
        return;

    _editing = false;

#ifdef SUPPORTS_SOFTKEYBOARD
    VInputManager::GetSoftkeyboardAdapter().Hide();
    VInputManager::GetSoftkeyboardAdapter().SetCurrentRecipient(NULL);
#else
    IMEAdapter::setCompositionMode(IMEAdapter::CompositionMode::Auto);
#endif

    if (!_promptText.empty())
        updateText();

    StageInst->getCaret()->removeFromParent();
    StageInst->getSelectionShape()->removeFromParent();
}

void InputTextField::onTouchBegin(EventContext * context)
{
    if (!_editing || _textField->_charPositions->size() <= 1)
        return;

    clearSelection();

    hkvVec2 v = context->getInput()->getPosition();
    v = globalToLocal(v);
    const TextField::CharPosition& cp = getCharPosition(v);

    adjustCaret(cp, true);

    context->captureTouch();
}

void InputTextField::onTouchMove(EventContext * context)
{
    hkvVec2 v = context->getInput()->getPosition();
    v = globalToLocal(v);

    const TextField::CharPosition& cp = getCharPosition(v);
    if (cp.charIndex != _caretPosition)
        adjustCaret(cp);
}

void InputTextField::handleSpecialKey(int keyId, bool shift, bool ctrl)
{
    switch (keyId)
    {
    case VGLK_BACKSP:
    {
        if (_selectionStart == _caretPosition && _caretPosition > 0)
            _selectionStart = _caretPosition - 1;
        replaceSelection("");
        break;
    }

    case VGLK_DEL:
    {
        if (_selectionStart == _caretPosition && _caretPosition < (int)(_textField->_charPositions->size() - 1))
            _selectionStart = _caretPosition + 1;
        replaceSelection("");
        break;
    }

    case VGLK_LEFT:
    {
        if (!shift)
            clearSelection();
        if (_caretPosition > 0)
        {
            const TextField::CharPosition& cp = getCharPosition(_caretPosition - 1);
            adjustCaret(cp, !shift);
        }
        break;
    }

    case VGLK_RIGHT:
    {
        if (!shift)
            clearSelection();
        if (_caretPosition < (int)(_textField->_charPositions->size() - 1))
        {
            const TextField::CharPosition& cp = getCharPosition(_caretPosition + 1);
            adjustCaret(cp, !shift);
        }
        break;
    }

    case VGLK_UP:
    {
        if (!shift)
            clearSelection();

        const TextField::CharPosition& cp = getCharPosition(_caretPosition);
        if (cp.lineIndex == 0)
            return;

        TextField::LineInfo* line = _textField->_lines[cp.lineIndex - 1];
        const TextField::CharPosition& cp2 = getCharPosition(hkvVec2(StageInst->getCaret()->getX(), line->y + _textField->getY()));
        adjustCaret(cp2, !shift);
        break;
    }

    case VGLK_DOWN:
    {
        if (!shift)
            clearSelection();

        const TextField::CharPosition& cp = getCharPosition(_caretPosition);
        if (cp.lineIndex == _textField->_lines.size() - 1)
            adjustCaret(_textField->_charPositions->back(), !shift);
        else
        {
            TextField::LineInfo* line = _textField->_lines[cp.lineIndex + 1];
            adjustCaret(getCharPosition(hkvVec2(StageInst->getCaret()->getX(), line->y + _textField->getY())), !shift);
        }
        break;
    }

    case VGLK_HOME:
    {
        if (!shift)
            clearSelection();

        const TextField::CharPosition& cp = getCharPosition(_caretPosition);
        TextField::LineInfo* line = _textField->_lines[cp.lineIndex];
        const TextField::CharPosition& cp2 = getCharPosition(hkvVec2(INT_MIN, line->y + _textField->getY()));
        adjustCaret(cp2, !shift);
        break;
    }

    case VGLK_END:
    {
        if (!shift)
            clearSelection();

        const TextField::CharPosition& cp = getCharPosition(_caretPosition);
        TextField::LineInfo* line = _textField->_lines[cp.lineIndex];
        const TextField::CharPosition& cp2 = getCharPosition(hkvVec2(INT_MAX, line->y + _textField->getY()));
        adjustCaret(cp2, !shift);

        break;
    }
    //Select All
    case VGLK_A:
    {
        if (ctrl)
        {
            _selectionStart = 0;
            adjustCaret(getCharPosition(INT_MAX));
        }
        break;
    }

    //Copy
    case VGLK_C:
    {
        if (ctrl && !_displayAsPassword)
        {
            std::string s = getSelection();
            //if (!s.empty())
            //  doCopy(s);
        }
        break;
    }

    //Paste
    case VGLK_V:
    {
        if (ctrl)
        {
            //doPaste();
        }
        break;
    }

    //Cut
    case VGLK_X:
    {
        if (ctrl && !_displayAsPassword)
        {
            std::string s = getSelection();
            if (!s.empty())
            {
                //doCopy(s);
                replaceSelection("");
            }
        }
        break;
    }

    case VGLK_ENTER:
    case VGLK_KP_ENTER:
    {
        if (_textField->_singleLine)
        {
            dispatchEvent(UIEventType::Submit);
            return;
        }
        else
            replaceSelection("\n");
        break;
    }
    }
}

void InputTextField::onKeyDown(EventContext * context)
{
    if (!_editing || context->isDefaultPrevented())
        return;

    InputEvent* evt = context->getInput();
    handleSpecialKey(evt->getKeyCode(), evt->isShiftDown(), evt->isCtrlDown());

    std::string c = evt->getKeyName();
    if (!c.empty())
    {
        if (evt->isCtrlDown())
            return;

        replaceSelection(c);
    }
    else
    {
        if (!IMEAdapter::getCompositionString().empty())
            updateText();
    }
}

NS_FGUI_END