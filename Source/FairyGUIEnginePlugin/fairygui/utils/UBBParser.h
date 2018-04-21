#ifndef __UBBPARSER_H__
#define __UBBPARSER_H__

#include "FGUIMacros.h"

NS_FGUI_BEGIN

#define UBB_TAG_HANDLER(__selector__,__target__, ...) std::bind(&__selector__,__target__, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, ##__VA_ARGS__)

class FGUI_IMPEXP UBBParser
{
public:
    static UBBParser defaultParser;

    UBBParser();
    virtual ~UBBParser();

    std::string parse(const char *text);

    int defaultImgWidth;
    int defaultImgHeight;

protected:
    virtual void onTag_URL(const std::string& tagName, bool end, const std::string& attr, std::string& replacement);
    virtual void onTag_IMG(const std::string& tagName, bool end, const std::string& attr, std::string& replacement);
    virtual void onTag_Simple(const std::string& tagName, bool end, const std::string& attr, std::string& replacement);
    virtual void onTag_COLOR(const std::string& tagName, bool end, const std::string& attr, std::string& replacement);
    virtual void onTag_FONT(const std::string& tagName, bool end, const std::string& attr, std::string& replacement);
    virtual void onTag_SIZE(const std::string& tagName, bool end, const std::string& attr, std::string& replacement);
    virtual void onTag_ALIGN(const std::string& tagName, bool end, const std::string& attr, std::string& replacement);

    void getTagText(std::string& out, bool remove);

    typedef std::function<void(const std::string& tagName, bool end, const std::string& attr, std::string& replacement)> TagHandler;
    std::unordered_map<std::string, TagHandler> _handlers;

    const char* _pString;
    ssize_t _readPos;
};

NS_FGUI_END

#endif