#include "ChangePageAction.h"
#include "GComponent.h"

NS_FGUI_BEGIN

void ChangePageAction::setup(TXMLElement * xml)
{
    ControllerAction::setup(xml);

    const char *p;
    p = xml->Attribute("objectId");
    if (p)
        objectId = p;
    p = xml->Attribute("controller");
    if (p)
        controllerName = p;
    p = xml->Attribute("targetPage");
    if (p)
        targetPage = p;
}

void ChangePageAction::enter(GController * controller)
{
    if (controllerName.empty())
        return;

    GComponent* gcom;
    if (!objectId.empty())
        gcom = controller->getParent()->getChildById(objectId)->as<GComponent>();
    else
        gcom = controller->getParent();
    if (gcom != nullptr)
    {
        GController* cc = gcom->getController(controllerName);
        if (cc != nullptr && cc != controller && !cc->isChanging())
            cc->setSelectedPageId(targetPage);
    }
}

void ChangePageAction::leave(GController * controller)
{
}

NS_FGUI_END