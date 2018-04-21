#include "GearText.h"
#include "GObject.h"
#include "UIPackage.h"
#include "GController.h"
#include "utils/ToolSet.h"
#include "utils/ActionUtils.h"

NS_FGUI_BEGIN


GearText::GearText(GObject * owner) :GearBase(owner)
{

}

GearText::~GearText()
{
}

void GearText::init()
{
    _default = _owner->getText();
    _storage.clear();
}

void GearText::addStatus(const std::string&  pageId, const std::string& value)
{
    if (pageId.length() == 0)
        _default = value;
    else
        _storage[pageId] = value;
}

void GearText::apply()
{
    _owner->_gearLocked = true;

    auto it = _storage.find(_controller->getSelectedPageId());
    if (it != _storage.end())
        _owner->setText(it->second);
    else
        _owner->setText(_default);

    _owner->_gearLocked = false;
}

void GearText::updateState()
{
    _storage[_controller->getSelectedPageId()] = _owner->getText();
}

NS_FGUI_END