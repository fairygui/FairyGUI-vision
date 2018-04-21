#ifndef __INPUTFIELD_H__
#define __INPUTFIELD_H__

#include "FGUIMacros.h"
#include "RichTextField.h"

NS_FGUI_BEGIN

class SoftKeyboardHelper;

class FGUI_IMPEXP InputTextField : public RichTextField
{
public:
    CREATE_FUNC(InputTextField);

    virtual const std::string& getText() override { return _text; }
    virtual void setText(const std::string& value) override;
    virtual void applyTextFormat() override;

    bool isDisplayAsPassword() const { return _displayAsPassword; }
    void setDisplayAsPassword(bool value);

    void setPromptText(const std::string& value);
    void setMaxLength(int value) { _maxLength = value; }
    void setRestrict(const std::string& value);

    void replaceSelection(const std::string& value);
    void replaceText(const std::string& value);

    virtual void update(float dt) override;

protected:
    InputTextField();
    virtual ~InputTextField();

protected:
    virtual void onSizeChanged(bool widthChanged, bool heightChanged) override;

private:
    void updateText();
    std::string encodePasswordText(const std::string& value);
    const TextField::CharPosition& getCharPosition(int caretIndex);
    const TextField::CharPosition& getCharPosition(const hkvVec2& location);
    hkvVec2 getCharLocation(const TextField::CharPosition& cp);
    void adjustCaret(const TextField::CharPosition& cp, bool moveSelectionHeader = false);
    void updateCaret(bool forceUpdate = false);
    hkvVec2 moveContent(const hkvVec2& delta, bool forceUpdate);
    void clearSelection();
    std::string getSelection();
    void updateSelection(const TextField::CharPosition& cp);
    void onPostBuilt();
    void handleSpecialKey(int keyId, bool shift, bool ctrl);

    void onFocusIn(EventContext* context);
    void onFocusOut(EventContext* context);
    void onTouchBegin(EventContext* context);
    void onTouchMove(EventContext* context);
    void onKeyDown(EventContext* context);

    std::string _text;
    bool _editable;
    int _maxLength;
    bool _displayAsPassword;
    std::string _promptText;

    bool _editing;
    int _caretPosition;
    int _selectionStart;
    int _composing;
#ifdef SUPPORTS_SOFTKEYBOARD
    SoftKeyboardHelper* _softkeyBoardHelper;
#endif

    static float _nextBlink;

    friend class TextField;
    friend class SoftKeyboardHelper;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(InputTextField);
};

NS_FGUI_END

#endif
