#ifndef __RENDERCONTEXT_H__
#define __RENDERCONTEXT_H__

#include "FGUIMacros.h"

NS_FGUI_BEGIN

class FGUI_IMPEXP RenderContext
{
public:
    RenderContext();

	void begin();
	void end();
	void enterClipping(const VRectanglef& rect);
	void leaveClipping();
    void enableClipping(bool value);

	struct ClipInfo
	{
		VRectanglef rect;
	};

	std::vector<ClipInfo> _clipStack;

	bool clipped;
	ClipInfo clipInfo;

	float alpha;
	bool grayed;

	IVRender2DInterface* getRenderer() const { return _renderer; }
	const VSimpleRenderState_t& getRenderState() const { return _renderState; }

private:
	IVRender2DInterface* _renderer;
	VSimpleRenderState_t _renderState;
};

NS_FGUI_END

#endif