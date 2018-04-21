#include "ControllerAction.h"
#include "GController.h"
#include "ChangePageAction.h"
#include "PlayTransitionAction.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

ControllerAction * ControllerAction::createAction(const char* type)
{
    if (strcmp(type, "play_transition") == 0)
        return new PlayTransitionAction();
    else if (strcmp(type, "change_page") == 0)
        return new ChangePageAction();
    else
        return nullptr;
}

ControllerAction::ControllerAction()
{
}

ControllerAction::~ControllerAction()
{
}

void ControllerAction::run(GController * controller, const std::string & prevPage, const std::string & curPage)
{
    if ((fromPage.empty() || std::find(fromPage.cbegin(), fromPage.cend(), prevPage) != fromPage.cend())
        && (toPage.empty() || std::find(toPage.cbegin(), toPage.cend(), curPage) != toPage.cend()))
        enter(controller);
    else
        leave(controller);
}

void ControllerAction::setup(TXMLElement * xml)
{
    const char* p;
    p = xml->Attribute("fromPage");
    if (p)
        ToolSet::splitString(p, ',', fromPage);
    p = xml->Attribute("toPage");
    if (p)
        ToolSet::splitString(p, ',', toPage);
}

NS_FGUI_END
