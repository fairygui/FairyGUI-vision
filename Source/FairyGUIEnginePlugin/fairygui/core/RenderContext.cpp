#include "RenderContext.h"
#include "GRoot.h"
#include "utils/ToolSet.h"

#define TRIAL_VERISON 1

NS_FGUI_BEGIN

RenderContext::RenderContext()
{
    _renderState = VSimpleRenderState_t(VIS_TRANSP_ALPHA, RENDERSTATEFLAG_ALWAYSVISIBLE | RENDERSTATEFLAG_FRONTFACE | RENDERSTATEFLAG_USESCISSORTEST | RENDERSTATEFLAG_USEADDITIVEALPHA | RENDERSTATEFLAG_FILTERING);
}

void RenderContext::begin()
{
    alpha = 1;
    grayed = false;

    clipped = false;
    _clipStack.clear();

    _renderer = Vision::RenderLoopHelper.BeginOverlayRendering();
}

void RenderContext::end()
{
#ifdef TRIAL_VERISON
    Vision::Fonts.DebugFont().PrintText(_renderer, hkvVec2(2, 2), "FairyGUI Trial Version", V_RGBA_WHITE, _renderState);
#endif

    Vision::RenderLoopHelper.EndOverlayRendering();
}

void RenderContext::enterClipping(const VRectanglef& rect)
{
    _clipStack.push_back(clipInfo);

    VRectanglef clipRect;
    if (clipped)
        clipRect = ToolSet::intersection(clipInfo.rect, rect);
    else
        clipRect = rect;

    clipped = true;
    clipInfo.rect = clipRect;
    _renderer->SetScissorRect(&clipInfo.rect);
}

void RenderContext::leaveClipping()
{
    clipInfo = _clipStack.back();
    _clipStack.pop_back();
    clipped = !_clipStack.empty();
    if (clipped)
        _renderer->SetScissorRect(&clipInfo.rect);
    else
        _renderer->SetScissorRect(nullptr);
}

void RenderContext::enableClipping(bool value)
{
    if (value)
    {
        if (clipped)
            _renderer->SetScissorRect(&clipInfo.rect);
    }
    else if (clipped)
        _renderer->SetScissorRect(nullptr);
}

NS_FGUI_END