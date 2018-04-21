#ifndef __FAIRYMACROS_H__
#define __FAIRYMACROS_H__

#define NS_FGUI_BEGIN                     namespace fairygui {
#define NS_FGUI_END                       }
#define USING_NS_FGUI                     using namespace fairygui

#ifdef WIN32
#ifdef _WINDLL
#define FGUI_IMPEXP __declspec(dllexport)
#else
#define FGUI_IMPEXP __declspec(dllimport)
#endif

#elif defined (_VISION_IOS) || defined(_VISION_ANDROID) || defined(HK_PLATFORM_TIZEN)
#define FGUI_IMPEXP
#else
#error Undefined platform!
#endif

#include <Vision/Runtime/Engine/System/Vision.hpp>
#include <Vision/Runtime/EnginePlugins/VisionEnginePlugin/GUI/VMenuIncludes.hpp>
#include <Vision/Runtime/EnginePlugins/ThirdParty/FmodEnginePlugin/vFmodManager.hpp>

#ifndef MAX
#define MAX(x,y) hkvMath::Max<float>(x,y)
#endif

#ifndef MIN
#define MIN(x,y) hkvMath::Min<float>(x,y)
#endif

#define StageInst FGUIManager::GlobalManager().getStage()
#define UIRoot FGUIManager::GlobalManager().getUIRoot()

#include "third_party/tinyxml2/tinyxml2.h"

typedef tinyxml2::XMLElement TXMLElement;
typedef tinyxml2::XMLDocument TXMLDocument;

#include "FieldTypes.h"
#include "third_party/cc/CCValue.h"
#include "third_party/cc/CCVector.h"
#include "third_party/cc/CCMap.h"
#include "third_party/cc/CCRefPtr.h"
#include "third_party/cc/CCRef.h"


#endif