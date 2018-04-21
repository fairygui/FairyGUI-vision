#ifndef __FAIRYGUI_MANAGER_H__
#define __FAIRYGUI_MANAGER_H__

#include "FGUIMacros.h"
#include "core/Stage.h"

NS_FGUI_BEGIN

class GRoot;
class RenderContext;
class BaseFont;

class FGUI_IMPEXP FGUIManager : public IVisCallbackHandler_cl
{
public:
    static FGUIManager& GlobalManager() { return g_GlobalManager; }

    FGUIManager();

    Stage* getStage();
    GRoot* getUIRoot();
    ActionManager* getActionManager();
    Scheduler* getScheduler();
    NTexture* getWhiteTexture();

    void setShowCursor(bool show);
    bool isShowCursor() const;
    void setCursor(const char* filePath);

    void registerFont(const char* fontName, const char* fontFilePath, int fontSize = 0, bool simulateOutline = false);
    BaseFont* getFontByName(const std::string& fontName);

    int getFrameCount() const { return _frameCount; }

    void OneTimeInit();
    void OneTimeDeInit();

    // switch to play-the-game mode. When not in vForge, this is default
    void setPlayTheGame(bool bStatus);

    virtual void OnHandleCallback(IVisCallbackDataObject_cl* pData) override;

private:
    void cleanupResources();

    Stage* _stage;
    GRoot* _groot;
    Scheduler* _scheduler;
    ActionManager* _actionManager;
    NTexture* _whiteTexture;

    RenderContext* _renderContext;
    hkUint32 _frameCount;
    bool _playingTheGame;

    VGUIMainContextPtr _spGuiContext;
    VCursorPtr _spCursor;

    std::unordered_map<std::string, BaseFont*> _fonts;

    static FGUIManager g_GlobalManager;
};

inline Stage * FGUIManager::getStage()
{
    return _stage;
}

inline GRoot * FGUIManager::getUIRoot()
{
    return _groot;
}

inline ActionManager * FGUIManager::getActionManager()
{
    return _actionManager;
}

inline Scheduler * FGUIManager::getScheduler()
{
    return _scheduler;
}

inline NTexture * FGUIManager::getWhiteTexture()
{
    return _whiteTexture;
}

NS_FGUI_END

#endif