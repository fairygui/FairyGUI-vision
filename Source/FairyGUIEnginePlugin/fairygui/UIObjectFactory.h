#ifndef __UIOBJECTFACTORY_H__
#define __UIOBJECTFACTORY_H__

#include "FGUIMacros.h"

NS_FGUI_BEGIN

class GLoader;
class GComponent;
class PackageItem;
class GObject;

class FGUI_IMPEXP UIObjectFactory
{
public:
    typedef std::function<GComponent*()> GComponentCreator;
    typedef std::function<GLoader*()> GLoaderCreator;

    static void setPackageItemExtension(const std::string& url, GComponentCreator creator);
    static GObject* newObject(PackageItem* pi);
    static GObject* newObject(const std::string& type);

    static void setLoaderExtension(GLoaderCreator creator);

private:
    static void resolvePackageItemExtension(PackageItem* pi);

    static std::unordered_map<std::string, GComponentCreator> _packageItemExtensions;
    static GLoaderCreator _loaderCreator;

    friend class UIPackage;
};

NS_FGUI_END

#endif
