#ifndef __STAGE_H__
#define __STAGE_H__

#include "FGUIMacros.h"
#include "DisplayObject.h"

NS_FGUI_BEGIN

class RenderContext;
class TouchInfo;
class Shape;
class SelectionShape;

class FGUI_IMPEXP Stage : public DisplayObject
{
public:
    CREATE_FUNC(Stage);

    const hkvVec2& getTouchPosition();
    const hkvVec2& getTouchPosition(int touchId);
    DisplayObject* getTouchTarget();
    void addTouchMonitor(int touchId, Node * target);
    void removeTouchMonitor(Node * target);
    void cancelClick(int touchId);
    void setFocus(DisplayObject* value);
    InputEvent* getRecentInput() const { return _recentInput; }

    Shape* getCaret() const { return _caret; }
    SelectionShape* getSelectionShape() { return _selectionShape; }

    void playSound(const std::string& url, float volumeScale = 1);
    bool isSoundEnabled() const { return _soundEnabled; }
    void setSoundEnabled(bool value);
    float getSoundVolumeScale() const { return _soundVolumeScale; }
    void setSoundVolumeScale(float value);

    virtual void update(float dt) override;

protected:
    Stage();
    virtual ~Stage();

private:
    TouchInfo* getTouch(int touchId, bool createIfNotExisits);
    void updateEvent(TouchInfo* ti);
    void handleRollOver(TouchInfo* touch);
    void setBegin(TouchInfo* touch);
    void setEnd(TouchInfo* touch);
    void setMove(TouchInfo* touch);
    DisplayObject* clickTest(TouchInfo* touch);
    void onKeyDown(int keyId, int modifiers);

    void parseHit();
    void handleMouseInput();
    void handleTouchInput();
    void handleKeyboardInput();

    IVMultiTouchInput* _multiTouchInput;
    VMousePC* _mouseInput;

    std::vector<TouchInfo*> _touches;
    hkvVec2 _touchPosition;
    int _touchCount;
    WeakPtr _touchTarget;
    WeakPtr _focused;
    float* _keyStatus;
    InputEvent* _recentInput;
    bool _capsLockOn;
    bool _soundEnabled;
    float _soundVolumeScale;

    Shape* _caret;
    SelectionShape* _selectionShape;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(Stage);
};

NS_FGUI_END

#endif