#include "FGUIManager.h"
#include "UIPackage.h"
#include "GRoot.h"
#include "core/RenderContext.h"
#include "core/BaseFont.h"
#include "core/BitmapFont.h"
#include "core/NativeFont.h"
#include "third_party/cc/CCAutoreleasePool.h"

#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/Scripting/VScriptManager.hpp>

NS_FGUI_BEGIN

//extern "C" int luaopen_FairyGUIModule(lua_State *);

FGUIManager FGUIManager::g_GlobalManager;

FGUIManager::FGUIManager() :
    _whiteTexture(nullptr),
    _scheduler(nullptr),
    _actionManager(nullptr),
    _stage(nullptr),
    _groot(nullptr),
    _renderContext(nullptr),
    _frameCount(0),
    _playingTheGame(false)
{
}

void FGUIManager::OneTimeInit()
{
    _whiteTexture = new NTexture();

    _scheduler = new Scheduler();
    _actionManager = new ActionManager();

    _scheduler->scheduleUpdate(_actionManager, Scheduler::PRIORITY_SYSTEM, false);

    _stage = Stage::create();
    _stage->retain();

    _groot = GRoot::create();
    _groot->retain();
    _stage->addChild(_groot->displayObject());

    _spGuiContext = new VGUIMainContext(&VGUIManager::GlobalManager());

    _renderContext = new RenderContext();

    Vision::Callbacks.OnEditorModeChanged += this;
    Vision::Callbacks.OnBeforeSceneLoaded += this;
    Vision::Callbacks.OnAfterSceneLoaded += this;
    Vision::Callbacks.OnUpdateSceneBegin += this;
    Vision::Callbacks.OnWorldDeInit += this;

    Vision::Callbacks.OnFrameUpdatePreRender += this;
    Vision::Callbacks.OnRenderHook += this;
    Vision::Callbacks.OnVideoChanged += this;
    Vision::Callbacks.OnEngineDeInit += this;
    IVScriptManager::OnRegisterScriptFunctions += this;
}

void FGUIManager::OneTimeDeInit()
{
    cleanupResources();

    Vision::Callbacks.OnEditorModeChanged -= this;
    Vision::Callbacks.OnBeforeSceneLoaded -= this;
    Vision::Callbacks.OnAfterSceneLoaded -= this;
    Vision::Callbacks.OnUpdateSceneBegin -= this;
    Vision::Callbacks.OnWorldDeInit -= this;

    Vision::Callbacks.OnFrameUpdatePreRender -= this;
    Vision::Callbacks.OnRenderHook -= this;
    Vision::Callbacks.OnVideoChanged -= this;
    Vision::Callbacks.OnEngineDeInit -= this;
    IVScriptManager::OnRegisterScriptFunctions -= this;
}

void FGUIManager::cleanupResources()
{
    if (_actionManager)
        _actionManager->removeAllActions();
    if (_scheduler)
        _scheduler->pauseAllTargets();

    CC_SAFE_RELEASE_NULL(_groot);
    CC_SAFE_RELEASE_NULL(_stage);
    CC_SAFE_RELEASE_NULL(_actionManager);
    CC_SAFE_RELEASE_NULL(_scheduler);
    CC_SAFE_RELEASE_NULL(_whiteTexture);
    CC_SAFE_DELETE(_renderContext);

    for (auto &it : _fonts)
        delete it.second;
    _fonts.clear();

    _spGuiContext = NULL;
    _spCursor = NULL;

    PoolManager::destroyInstance();

    UIPackage::removeAllPackages();
}

// switch to play-the-game mode
void FGUIManager::setPlayTheGame(bool bStatus)
{
    if (_playingTheGame == bStatus)
        return;

    _playingTheGame = bStatus;

    if (_playingTheGame)
    {
        setCursor("Textures/vapp_cursor.tga");
        // Play the game mode is started
        //Vision::Message.Add(1, "Play the game mode has been started");
    }
    else
    {
        //the play the game mode has been stopped.
        //clean up all your game specific instances, like e.g. particle effects
        //VisParticleGroupManager_cl::GlobalManager().Instances().Purge();
    }
}

void FGUIManager::setShowCursor(bool show)
{
    _spGuiContext->SetActivate(show);
}

bool FGUIManager::isShowCursor() const
{
    return _spGuiContext->IsActive();
}

void FGUIManager::setCursor(const char* filePath)
{
    _spCursor = _spGuiContext->GetManager()->LoadCursorResource(filePath);
    _spGuiContext->SetCurrentCursor(_spCursor);
}

void FGUIManager::registerFont(const char* fontName, const char* fontFilePath, int fontSize, bool simulateOutline)
{
    auto it = _fonts.find(fontName);
    if (it == _fonts.cend())
    {
        NativeFont* font = new NativeFont(fontName, fontFilePath, fontSize, simulateOutline);
        _fonts[fontName] = font;
    }
    else
        CCLOGERROR("Font %s already registered!", fontName);
}

BaseFont * FGUIManager::getFontByName(const std::string & fontName)
{
    if (fontName.compare(0, 5, "ui://") == 0)
    {
        PackageItem* pi = UIPackage::getItemByURL(fontName);
        if (pi != nullptr)
        {
            if (!pi->decoded)
                pi->load();
            if (pi->bitmapFont)
                return pi->bitmapFont;
        }
    }

    BaseFont* ret;
    auto it = _fonts.find(fontName);
    if (it == _fonts.cend())
    {
        it = _fonts.find("<default>");
        if (it == _fonts.cend())
        {
            ret = new NativeFont(fontName, fontName, 0, false);
            _fonts[fontName] = ret;
            return ret;
        }
        else
            return it->second;
    }
    else
        return it->second;
}

void FGUIManager::OnHandleCallback(IVisCallbackDataObject_cl * pData)
{
    if (pData->m_pSender == &Vision::Callbacks.OnFrameUpdatePreRender)
    {
        _frameCount++;

        float dt = Vision::GetTimer()->GetTimeDifference();
        getScheduler()->update(dt);
        _stage->update(dt);

        PoolManager::getInstance()->getCurrentPool()->clear();
    }
    else if (pData->m_pSender == &Vision::Callbacks.OnRenderHook)
    {
        VisRenderHookDataObject_cl* pRHDO = static_cast<VisRenderHookDataObject_cl*>(pData);
        if (pRHDO->m_iEntryConst != VRH_GUI)
            return;

        _renderContext->begin();
        _stage->onRender(_renderContext);
        _renderContext->end();
    }
    else if (pData->m_pSender == &Vision::Callbacks.OnVideoChanged)
    {
        VisVideoChangedDataObject_cl *pVideo = (VisVideoChangedDataObject_cl*)pData;
        _stage->setSize((float)pVideo->m_pConfig->m_iXRes, (float)pVideo->m_pConfig->m_iYRes);
    }
    else if (pData->m_pSender == &Vision::Callbacks.OnEngineDeInit)
    {
        OneTimeDeInit();
    }
    else if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneBegin)
    {
        //This callback will be triggered at the beginning of every frame
        //You can add your own per frame logic here
        // [...]
        /*if (m_bPlayingTheGame)
        {
        Vision::Message.Print(1, 200, 100, "The game is running");
        }*/
        return;
    }

    else if (pData->m_pSender == &Vision::Callbacks.OnEditorModeChanged)
    {
        // when vForge switches back from EDITORMODE_PLAYING_IN_GAME, turn off our play the game mode
        if (((VisEditorModeChangedDataObject_cl *)pData)->m_eNewMode != VisEditorManager_cl::EDITORMODE_PLAYING_IN_GAME)
            setPlayTheGame(false);
        return;
    }

    else if (pData->m_pSender == &Vision::Callbacks.OnBeforeSceneLoaded)
    {
        //here you can add you specific code before the scene is loaded
        return;
    }

    else if (pData->m_pSender == &Vision::Callbacks.OnAfterSceneLoaded)
    {
        //gets triggered when the play-the-game vForge is started or outside vForge after loading the scene
        if (Vision::Editor.IsPlayingTheGame())
            setPlayTheGame(true);
        return;
    }

    else if (pData->m_pSender == &Vision::Callbacks.OnWorldDeInit)
    {
        // this is important when running outside vForge
        setPlayTheGame(false);
        return;
    }
    else if (pData->m_pSender == &IVScriptManager::OnRegisterScriptFunctions)
    {
        /*(IVScriptManager* pSM = Vision::GetScriptManager();
        if(pSM)
        {
            lua_State* pLuaState = ((VScriptResourceManager*)pSM)->GetMasterState();
            if(pLuaState)
            {
                //luaopen_TestClassModule(pLuaState);
                lua_getglobal(pLuaState, "FGUI");
                int iType = lua_type(pLuaState, -1);
                lua_pop(pLuaState, 1);

                if(iType!=LUA_TUSERDATA)
                {
                    luaopen_FairyGUIModule(pLuaState);
                    int iRetParams = LUA_CallStaticFunction(pLuaState,"FairyGUIModule","FGUIManager","Cast","v>v", &FGUIManager::GlobalManager());
                    if (iRetParams==1)
                    {
                        if(lua_isnil(pLuaState, -1))
                        {
                            lua_pop(pLuaState, iRetParams);
                        }
                        else
                        {
                            lua_setglobal(pLuaState, "FGUI");
                            return;
                        }
                    }
                }
            }
            else
                hkvLog::Error("Unable to create Lua Racer Module, lua_State is NULL!");
        }*/
        return;
    }
}

NS_FGUI_END

