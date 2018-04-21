/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2015 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */


//  Basic Plugin Framework to house your own components
//

#include "FairyGUIEnginePluginPCH.h"
#include "myComponent.h"
#include "FairyGUI.h"

#include <Common/Base/KeyCode.h>

// use plugins if supported
VIMPORT IVisPlugin_cl* GetEnginePlugin_vFmodEnginePlugin();
#if defined( HAVOK_PHYSICS_2012_KEYCODE )
VIMPORT IVisPlugin_cl* GetEnginePlugin_vHavok();
#endif
#if defined( HAVOK_AI_KEYCODE )
VIMPORT IVisPlugin_cl* GetEnginePlugin_vHavokAi();
#endif
#if defined( HAVOK_BEHAVIOR_KEYCODE )
VIMPORT IVisPlugin_cl* GetEnginePlugin_vHavokBehavior();
#endif

//============================================================================================================
//  Set up the Plugin Class
//============================================================================================================
class FairyGUIEnginePlugin : public IVisPlugin_cl
{
public:

  void OnInitEnginePlugin();    
  void OnDeInitEnginePlugin();  

  const char *GetPluginName()
  {
    return "FairyGUIEnginePlugin";  // must match DLL name
  }
};

//  global plugin instance
FairyGUIEnginePlugin g_myComponents;

//--------------------------------------------------------------------------------------------
//  create a global instance of a VModule class
//  note: g_myComponentModule is defined in stdfx.h
//--------------------------------------------------------------------------------------------
DECLARE_THIS_MODULE(g_myComponentModule, MAKE_VERSION(1,0),
                    "FairyGUI", 
                    "fairygui.com",
                    "FairyGUI plugin", &g_myComponents);


//--------------------------------------------------------------------------------------------
//  Use this to get and initialize the plugin when you link statically
//--------------------------------------------------------------------------------------------
VEXPORT IVisPlugin_cl* GetEnginePlugin_FairyGUIEnginePlugin(){  return &g_myComponents; }


#if (defined _DLL) || (defined _WINDLL)

  //  The engine uses this to get and initialize the plugin dynamically
  VEXPORT IVisPlugin_cl* GetEnginePlugin(){return GetEnginePlugin_FairyGUIEnginePlugin();}

#endif // _DLL or _WINDLL


//============================================================================================================
//  Initialize our plugin.
//============================================================================================================
//  Called when the plugin is loaded
//  We add our component initialize data here
void FairyGUIEnginePlugin::OnInitEnginePlugin()
{
  hkvLog::Info("FairyGUIEnginePlugin:OnInitEnginePlugin()");
  Vision::RegisterModule(&g_myComponentModule);

  
// load plugins if supported
#if defined( HAVOK_PHYSICS_2012_KEYCODE )
  VISION_PLUGIN_ENSURE_LOADED(vHavok);
#endif
#if defined( HAVOK_AI_KEYCODE )
  VISION_PLUGIN_ENSURE_LOADED(vHavokAi);
#endif
#if defined( HAVOK_BEHAVIOR_KEYCODE )
  VISION_PLUGIN_ENSURE_LOADED(vHavokBehavior);
#endif
  
  VISION_PLUGIN_ENSURE_LOADED(vFmodEnginePlugin);

  // In some cases the compiler optimizes away the full class from the plugin since it seems to be dead code. 
  // One workaround to prevent this is to add the following helper macro into the plugin initialization code:
  FORCE_LINKDYNCLASS( MyComponent );

  // [...]

  // Start our component managers and game manager here....
  fairygui::FGUIManager::GlobalManager().OneTimeInit();
  MyComponent_ComponentManager::GlobalManager().OneTimeInit();
  // [...]

  //---------------------------------------------------------------------------------------------------------
  // register the action module with the vision engine action manager
  // only after that will the action become available in the console.
  //---------------------------------------------------------------------------------------------------------
  VActionManager * pManager = Vision::GetActionManager ();
  pManager->RegisterModule ( &g_myComponentModule );

  // Set to true to open the console at startup and print some data to the display
  Vision::GetConsoleManager()->Show( false );
  pManager->Print( "Welcome to the console!" );
  pManager->Print( "This module is called '%s'", g_myComponentModule.GetName() );
  pManager->Print( "Type in 'help' for a list of all actions" );
  pManager->Print( "Type in 'fgui cursor' show/hide the cursor." );
}

// Called before the plugin is unloaded
void FairyGUIEnginePlugin::OnDeInitEnginePlugin()
{
  hkvLog::Info("FairyGUIEnginePlugin:OnDeInitEnginePlugin()");
    
  // Close our component managers here....
  MyComponent_ComponentManager::GlobalManager().OneTimeDeInit();
  fairygui::FGUIManager::GlobalManager().OneTimeDeInit();
  //  [...]
  
  // de-register component from action manager
  VActionManager * pManager = Vision::GetActionManager ();
  pManager->UnregisterModule( &g_myComponentModule );

  // de-register our module when the plugin is de-initialized
  Vision::UnregisterModule(&g_myComponentModule);
}

/*
 * Havok SDK - Base file, BUILD(#20150320)
 * 
 * Confidential Information of Havok.  (C) Copyright 1999-2015
 * Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
 * Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
 * rights, and intellectual property rights in the Havok software remain in
 * Havok and/or its suppliers.
 * 
 * Use of this software for evaluation purposes is subject to and indicates
 * acceptance of the End User licence Agreement for this product. A copy of
 * the license is included with this software and is also available from salesteam@havok.com.
 * 
 */
