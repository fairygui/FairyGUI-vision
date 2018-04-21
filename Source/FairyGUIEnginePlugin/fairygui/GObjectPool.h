#ifndef __GOBJECTPOOL_H__
#define __GOBJECTPOOL_H__

#include "FGUIMacros.h"

NS_FGUI_BEGIN

class GObject;

class FGUI_IMPEXP GObjectPool
{
public:
    GObjectPool();
    ~GObjectPool();

    GObject* getObject(const std::string& url);
    void returnObject(GObject* obj);

private:
    std::unordered_map<std::string, Vector<GObject*>> _pool;
};

NS_FGUI_END

#endif
