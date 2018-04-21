#ifndef __DRAGDROPMANAGER_H__
#define __DRAGDROPMANAGER_H__

#include "FGUIMacros.h"

NS_FGUI_BEGIN

class GLoader;
class EventContext;
class GRoot;

class FGUI_IMPEXP DragDropManager
{
public:
    DragDropManager(GRoot* root);
    virtual ~DragDropManager();

    GLoader* getAgent() const { return _agent; }
    bool isDragging() const;
    void startDrag(const std::string& icon, const Value& sourceData = Value::Null, int touchPointID = -1);
    void cancel();

private:
    void onDragEnd(EventContext* context);

    GLoader* _agent;
    Value _sourceData;
    GRoot* _groot;
};

NS_FGUI_END

#endif
