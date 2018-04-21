#include "GController.h"
#include "GComponent.h"
#include "utils/ToolSet.h"
#include "controller_action/ControllerAction.h"

NS_FGUI_BEGIN

GController::GController() :
    _changing(false),
    _autoRadioGroupDepth(false),
    _parent(nullptr),
    _selectedIndex(-1),
    _previousIndex(-1)
{
}

GController::~GController()
{
    for (auto &it : _actions)
        delete it;
}

void GController::setSelectedIndex(int value)
{
    if (_selectedIndex != value)
    {
        CCASSERT(value < (int)_pageIds.size(), "Invalid selected index");

        _changing = true;

        _previousIndex = _selectedIndex;
        _selectedIndex = value;
        _parent->applyController(this);

        dispatchEvent(UIEventType::Changed);

        _changing = false;
    }
}

const std::string& GController::getSelectedPage() const
{
    if (_selectedIndex == -1)
        return STD_STRING_EMPTY;
    else
        return _pageNames[_selectedIndex];
}

void GController::setSelectedPage(const std::string & value)
{
    int i = ToolSet::findInStringArray(_pageNames, value);
    if (i == -1)
        i = 0;
    setSelectedIndex(i);
}

const std::string& GController::getSelectedPageId() const
{
    if (_selectedIndex == -1)
        return STD_STRING_EMPTY;
    else
        return _pageIds[_selectedIndex];
}

void GController::setSelectedPageId(const std::string & value)
{
    int i = ToolSet::findInStringArray(_pageIds, value);
    if (i != -1)
        setSelectedIndex(i);
}

const std::string& GController::getPreviousPage() const
{
    if (_previousIndex == -1)
        return STD_STRING_EMPTY;
    else
        return _pageNames[_previousIndex];
}

const std::string& GController::getPreviousPageId() const
{
    if (_previousIndex == -1)
        return STD_STRING_EMPTY;
    else
        return _pageIds[_previousIndex];
}

int GController::getPageCount() const
{
    return (int)_pageIds.size();
}

bool GController::hasPage(const std::string & aName) const
{
    return ToolSet::findInStringArray(_pageNames, aName) != -1;
}

int GController::getPageIndexById(const std::string & value) const
{
    return ToolSet::findInStringArray(_pageIds, value);
}

const std::string& GController::getPageNameById(const std::string & value) const
{
    int i = ToolSet::findInStringArray(_pageIds, value);
    if (i != -1)
        return _pageNames[i];
    else
        return STD_STRING_EMPTY;
}

const std::string& GController::getPageId(int index) const
{
    return _pageIds[index];
}

void GController::setOppositePageId(const std::string & value)
{
    int i = ToolSet::findInStringArray(_pageIds, value);
    if (i > 0)
        setSelectedIndex(0);
    else if (_pageIds.size() > 1)
        setSelectedIndex(1);
}

void GController::addPage(const std::string & name)
{
    addPageAt(name, (int)_pageIds.size());
}

void GController::addPageAt(const std::string & name, int index)
{
    static int _nextPageId = 0;
    std::string nid = "_" + (_nextPageId++);
    if (index == (int)_pageIds.size())
    {
        _pageIds.push_back(nid);
        _pageNames.push_back(name);
    }
    else
    {
        _pageIds.insert(_pageIds.begin() + index, nid);
        _pageNames.insert(_pageNames.begin() + index, name);
    }
}

void GController::removePage(const std::string & name)
{
    auto it = std::find(_pageNames.cbegin(), _pageNames.cend(), name);
    if (it != _pageNames.end())
    {
        _pageIds.erase(_pageIds.begin() + (it - _pageIds.begin()));
        _pageNames.erase(it);
        if (_selectedIndex >= (int)_pageIds.size())
            setSelectedIndex(_selectedIndex - 1);
        else
            _parent->applyController(this);
    }
}

void GController::removePageAt(int index)
{
    _pageIds.erase(_pageIds.begin() + index);
    _pageNames.erase(_pageNames.begin() + index);
    if (_selectedIndex >= (int)_pageIds.size())
        setSelectedIndex(_selectedIndex - 1);
    else
        _parent->applyController(this);
}

void GController::clearPages()
{
    _pageIds.clear();
    _pageNames.clear();
    if (_selectedIndex != -1)
        setSelectedIndex(-1);
    else
        _parent->applyController(this);
}

void GController::runActions()
{
    if (_actions.empty())
        return;

    for (auto &it : _actions)
        it->run(this, getPreviousPageId(), getSelectedPageId());
}

void GController::setup(TXMLElement * xml)
{
    const char* p;

    p = xml->Attribute("name");
    if (p)
        _name = p;

    _autoRadioGroupDepth = xml->BoolAttribute("autoRadioGroupDepth");

    p = xml->Attribute("pages");
    if (p)
    {
        std::vector<std::string> elems;
        ToolSet::splitString(p, ',', elems);
        int cnt = (int)elems.size();
        for (int i = 0; i < cnt; i += 2)
        {
            _pageIds.push_back(elems[i]);
            _pageNames.push_back(elems[i + 1]);
        }
    }

    TXMLElement* cxml = xml->FirstChildElement("action");
    while (cxml)
    {
        ControllerAction* action = ControllerAction::createAction(cxml->Attribute("type"));
        action->setup(cxml);
        _actions.push_back(action);

        cxml = cxml->NextSiblingElement("action");
    }

    if (_parent != nullptr && _pageIds.size() > 0)
        _selectedIndex = 0;
    else
        _selectedIndex = -1;
}

NS_FGUI_END
