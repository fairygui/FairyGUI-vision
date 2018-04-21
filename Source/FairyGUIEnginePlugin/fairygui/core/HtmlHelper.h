#ifndef __HTMLHELPER_H__
#define __HTMLHELPER_H__

#include "FGUIMacros.h"
#include "TextFormat.h"

NS_FGUI_BEGIN

class IHtmlObject;
class DisplayObject;
class RichTextField;
class EventContext;

struct FGUI_IMPEXP HtmlParseOptions
{
    HtmlParseOptions();

    bool linkUnderline;
    VColorRef linkColor;
    VColorRef linkBgColor;
    VColorRef linkHoverBgColor;
    bool ignoreWhiteSpace;

    static bool defaultLinkUnderline;
    static VColorRef defaultLinkColor;
    static VColorRef defaultLinkBgColor;
    static VColorRef defaultLinkHoverBgColor;
};

class FGUI_IMPEXP HtmlElement
{
public:
    enum class Type
    {
        TEXT,
        IMAGE,
        LINK
    };

    HtmlElement(Type type);
    ~HtmlElement();

    void createObject(RichTextField* textField);

    Type type;
    std::string text;
    TextFormat format;
    int width;
    int height;
    int space;
    int status;
    hkvVec2 position;
    HtmlElement* link;
    IHtmlObject* htmlObject;
};

class FGUI_IMPEXP IHtmlObject
{
public:
    IHtmlObject() {}
    virtual ~IHtmlObject() {}

    virtual float getWidth() { return 0; }
    virtual float getHeight() { return 0; }
    virtual HtmlElement* getElement() { return nullptr; }

    virtual void create(RichTextField* owner, HtmlElement* element) {}
    virtual void setPosition(float x, float y) {}
    virtual void add() {}
    virtual void remove() {}
    virtual void release() {}
};

class GLoader;
class FGUI_IMPEXP HtmlImage : public IHtmlObject
{
public:
    HtmlImage();
    virtual ~HtmlImage();

    GLoader* getLoader() const { return _loader; }

    virtual float getWidth() override;
    virtual float getHeight() override;
    virtual HtmlElement* getElement() { return _element; }

    virtual void create(RichTextField* owner, HtmlElement* element) override;
    virtual void setPosition(float x, float y)  override;
    virtual void add() override;
    virtual void remove() override;
    virtual void release() override;

private:
    HtmlElement* _element;
    RichTextField* _owner;
    GLoader* _loader;
};

class SelectionShape;
class FGUI_IMPEXP HtmlLink : public IHtmlObject
{
public:
    HtmlLink();
    virtual ~HtmlLink();

    SelectionShape* getShape() const { return _shape; }

    virtual float getWidth() override;
    virtual float getHeight() override;
    virtual HtmlElement* getElement() { return _element; }

    virtual void create(RichTextField* owner, HtmlElement* element) override;
    virtual void setPosition(float x, float y)  override;
    virtual void add() override;
    virtual void remove() override;
    virtual void release() override;

    void setArea(int startLine, float startCharX, int endLine, float endCharX);

private:
    void onClick(EventContext* context);

    HtmlElement* _element;
    RichTextField* _owner;
    SelectionShape* _shape;
};

NS_FGUI_END

#endif
