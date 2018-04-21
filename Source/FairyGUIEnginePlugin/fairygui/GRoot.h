#ifndef __GROOT_H__
#define __GROOT_H__

#include "FGUIMacros.h"
#include "GComponent.h"
#include "FGUIManager.h"

NS_FGUI_BEGIN

class GGraph;
class Window;
class DragDropManager;

class FGUI_IMPEXP GRoot : public GComponent
{
public:
    CREATE_FUNC(GRoot);

    void setContentScaleFactor(int designResolutionX, int designResolutionY) { setContentScaleFactor(designResolutionX, designResolutionY, ScreenMatchMode::MatchWidthOrHeight); }
    void setContentScaleFactor(int designResolutionX, int designResolutionY, ScreenMatchMode screenMatchMode);

    DragDropManager* getDragDropManager() const { return _dragDropManager; }

    void showWindow(Window* win);
    void hideWindow(Window* win);
    void hideWindowImmediately(Window* win);
    void bringToFront(Window* win);
    void showModalWait();
    void closeModalWait();
    void closeAllExceptModals();
    void closeAllWindows();
    Window* getTopWindow();

    GObject* getModalWaitingPane();
    GGraph* getModalLayer();
    bool hasModalWindow();
    bool isModalWaiting();

    GObject* displayObjectToGObject(DisplayObject* obj);
    GObject* getTouchTarget();

    void showPopup(GObject* popup);
    void showPopup(GObject* popup, GObject* target, PopupDirection dir);
    void togglePopup(GObject* popup);
    void togglePopup(GObject* popup, GObject* target, PopupDirection dir);
    void hidePopup();
    void hidePopup(GObject* popup);
    bool hasAnyPopup();
    hkvVec2 getPoupPosition(GObject* popup, GObject* target, PopupDirection dir);

    void showTooltips(const std::string& msg);
    void showTooltipsWin(GObject* tooltipWin);
    void hideTooltips();

protected:
    GRoot();
    virtual ~GRoot();

protected:
    virtual void handleInit() override;

private:
    void createModalLayer();
    void adjustModalLayer();
    void closePopup(GObject* target);
    void checkPopups();
    void doShowTooltipsWin(float);
    
    void onWindowSizeChanged(EventContext* context);
    void onTouchBegin(EventContext* context);

    float _scaleFactor;
    GGraph* _modalLayer;
    GObject* _modalWaitPane;
    std::vector<WeakPtr> _popupStack;
    std::vector<WeakPtr> _justClosedPopups;
    GObject* _tooltipWin;
    GObject* _defaultTooltipWin;

    DragDropManager* _dragDropManager;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GRoot);
};

NS_FGUI_END

#endif
