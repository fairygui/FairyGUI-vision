#ifndef __GRICHTEXTFIELD_H__
#define __GRICHTEXTFIELD_H__

#include "FGUIMacros.h"
#include "GTextField.h"
#include "core/RichTextField.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP GRichTextField : public GTextField
{
public:
    CREATE_FUNC(GRichTextField);

protected:
    GRichTextField();
    virtual ~GRichTextField();

protected:
    virtual void handleInit() override;
    virtual void getTextFieldText() override;
    virtual void setTextFieldText() override;

private:
    RichTextField* _richTextField;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(GRichTextField);
};

NS_FGUI_END

#endif
