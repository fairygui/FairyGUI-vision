#ifndef __GCONTROLLER_H__
#define __GCONTROLLER_H__

#include "FGUIMacros.h"
#include "event/EventDispatcher.h"

NS_FGUI_BEGIN

class GComponent;
class ControllerAction;

class FGUI_IMPEXP GController : public EventDispatcher
{
public:
    GController();
    virtual ~GController();

    GComponent* getParent() const { return _parent; }
    void setParent(GComponent* value) { _parent = value; }

    const std::string getName() const { return _name; }
    void setName(const std::string& value) { _name = value; }

    bool isAutoRadioGroupDepth() const { return _autoRadioGroupDepth; }
    void setAutoRadioGroupDepth(bool value) { _autoRadioGroupDepth = value; }

    bool isChanging() const { return _changing; }

    int getSelectedIndex() const { return _selectedIndex; }
    void setSelectedIndex(int value);

    const std::string& getSelectedPage() const;
    void setSelectedPage(const std::string& value);

    const std::string& getSelectedPageId() const;
    void setSelectedPageId(const std::string& value);

    int getPrevisousIndex() const { return _previousIndex; }
    const std::string& getPreviousPage() const;
    const std::string& getPreviousPageId() const;

    int getPageCount() const;
    bool hasPage(const std::string& aName) const;
    int getPageIndexById(const std::string& value) const;
    const std::string& getPageNameById(const std::string& value) const;
    const std::string& getPageId(int index) const;
    void setOppositePageId(const std::string& value);

    void addPage(const std::string& name);
    void addPageAt(const std::string& name, int index);
    void removePage(const std::string& name);
    void removePageAt(int index);
    void clearPages();

    void runActions();

    void setup(TXMLElement* xml);

private:
    GComponent* _parent;
    int _selectedIndex;
    int _previousIndex;
    std::string _name;
    bool _autoRadioGroupDepth;
    bool _changing;

    std::vector<std::string> _pageIds;
    std::vector<std::string> _pageNames;
    std::vector<ControllerAction*> _actions;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GController);
};

NS_FGUI_END

#endif
