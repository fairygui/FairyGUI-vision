#ifndef __MOVIECLIP_H__
#define __MOVIECLIP_H__

#include "FGUIMacros.h"
#include "Image.h"

NS_FGUI_BEGIN

class MovieClip;

class FGUI_IMPEXP PlayState : public Ref
{
public:
    PlayState();

    int getCurrentFrame() const { return _currentFrame; }
    void setCurrentFrame(int value) { _currentFrame = value; }

    void update(MovieClip* mc, float dt);
    void rewind();
    void reset();
    
private:
    bool _reachEnding;
    bool _reversed;
    int _repeatedCount;
    bool _ignoreTimeScale;
    int _currentFrame;
    float _curFrameDelay;
    int _lastUpdateFrameId;
};

class FGUI_IMPEXP MovieClip : public Image
{
public:
    CREATE_FUNC(MovieClip);

    struct Frame
    {
    public:
        VRectanglef rect;
        float addDelay;
        VRectanglef uvRect;
        bool rotated;
    };

    void setInterval(float value) { _interval = value; }
    void setSwing(bool value) { _swing = value; }
    void setRepeatDelay(float value) { _repeatDelay = value; }
    void setData(NTexture* texture, const hkvArray<Frame>& frames, const VRectanglef& boundsRect);
    void clear();
    bool isPlaying() const { return _playing; }
    void setPlaying(bool value);
    int getCurrentFrame() const { return _currentFrame; }
    void setCurrentFrame(int value);
    void setPlaySettings() { setPlaySettings(0, -1, 0, -1); }
    void setPlaySettings(int start, int end, int times, int endAt, std::function<void()> completeCallback = nullptr);
    void drawFrame();

    virtual void update(float dt) override;

protected:
    MovieClip();
    virtual ~MovieClip();

protected:
    virtual void rebuild() override;

    void playCompleted(float);

    float _interval;
    bool _swing;
    float _repeatDelay;
    int _frameCount;
    hkvArray<Frame> _frames;
    RefPtr<PlayState> _playState;
    std::function<void()> _completeCallback;

    int _currentFrame;
    bool _playing;
    int _start;
    int _end;
    int _times;
    int _endAt;
    int _status; //0-none, 1-next loop, 2-ending, 3-ended
    bool _forceDraw;

    friend class PlayState;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(MovieClip);
};

NS_FGUI_END

#endif