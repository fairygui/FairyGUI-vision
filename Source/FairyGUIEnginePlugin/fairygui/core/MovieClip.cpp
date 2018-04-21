#include "MovieClip.h"
#include "FGUIManager.h"
#include "utils/ToolSet.h"

NS_FGUI_BEGIN

PlayState::PlayState() :
    _reachEnding(false),
    _reversed(false),
    _repeatedCount(0),
    _ignoreTimeScale(false),
    _currentFrame(0),
    _curFrameDelay(0),
    _lastUpdateFrameId(0)
{
}

void PlayState::update(MovieClip* mc, float dt)
{
    int frameId = FGUIManager::GlobalManager().getFrameCount();
    if (frameId - _lastUpdateFrameId != 1)
        //1、如果>1，表示不是连续帧了，说明刚启动（或者停止过），这里不能用流逝的时间了，不然会跳过很多帧
        //2、如果==0，表示在本帧已经处理过了，这通常是因为一个PlayState用于多个MovieClip共享，目的是多个MovieClip同步播放
        dt = 0;
    else if (_ignoreTimeScale)
        dt = Vision::GetTimer()->GetUnfilteredTimeDifference();
    _lastUpdateFrameId = frameId;

    _reachEnding = false;
    _curFrameDelay += dt;
    float interval = mc->_interval + mc->_frames[_currentFrame].addDelay + ((_currentFrame == 0 && _repeatedCount > 0) ? mc->_repeatDelay : 0);
    if (_curFrameDelay < interval)
        return;

    _curFrameDelay -= interval;
    if (_curFrameDelay > mc->_interval)
        _curFrameDelay = mc->_interval;

    if (mc->_swing)
    {
        if (_reversed)
        {
            _currentFrame--;
            if (_currentFrame <= 0)
            {
                _currentFrame = 0;
                _repeatedCount++;
                _reversed = !_reversed;
            }
        }
        else
        {
            _currentFrame++;
            if (_currentFrame > mc->_frameCount - 1)
            {
                _currentFrame = hkvMath::Max(0, mc->_frameCount - 2);
                _repeatedCount++;
                _reachEnding = true;
                _reversed = !_reversed;
            }
        }
    }
    else
    {
        _currentFrame++;
        if (_currentFrame > mc->_frameCount - 1)
        {
            _currentFrame = 0;
            _repeatedCount++;
            _reachEnding = true;
        }
    }
}

void PlayState::rewind()
{
    _currentFrame = 0;
    _curFrameDelay = 0;
    _reversed = false;
    _reachEnding = false;
}

void PlayState::reset()
{
    _currentFrame = 0;
    _curFrameDelay = 0;
    _repeatedCount = 0;
    _reachEnding = false;
    _reversed = false;
}

MovieClip::MovieClip() :
    _interval(0.1f),
    _swing(false),
    _repeatDelay(0),
    _frameCount(0),
    _currentFrame(0),
    _playing(true),
    _start(0),
    _end(0),
    _times(0),
    _endAt(0),
    _status(0),
    _forceDraw(false)
{
    _playState = new PlayState();
    setPlaySettings();
}

MovieClip::~MovieClip()
{
    CC_SAFE_RELEASE(_playState);
}

void MovieClip::setData(NTexture* texture, const hkvArray<Frame>& frames, const VRectanglef& boundsRect)
{
    _frames.Clear();
    _frames.PushBackRange(frames);
    _frameCount = frames.GetSize();
    _contentRect = boundsRect;

    if (_end == -1 || _end > _frameCount - 1)
        _end = _frameCount - 1;
    if (_endAt == -1 || _endAt > _frameCount - 1)
        _endAt = _frameCount - 1;
    _playState->rewind();

    if (_frameCount > 0)
        _graphics->setTexture(texture);
    else
        _graphics->setTexture(nullptr);
    onSizeChanged(true, true);
    _forceDraw = true;
    _completeCallback = nullptr;
}

void MovieClip::clear()
{
    _frameCount = 0;
    _completeCallback = nullptr;
    setTexture(nullptr);
    _graphics->clearMesh();
}

void MovieClip::setPlaying(bool value)
{
    _playing = value;
}

void MovieClip::setCurrentFrame(int value)
{
    if (_currentFrame != value)
    {
        _currentFrame = value;
        _playState->setCurrentFrame(value);
        if (_frameCount > 0)
            _forceDraw = true;
    }
}

void MovieClip::setPlaySettings(int start, int end, int times, int endAt, std::function<void()> completeCallback)
{
    _start = start;
    _end = end;
    if (_end == -1 || _end > _frameCount - 1)
        _end = _frameCount - 1;
    _times = times;
    _endAt = endAt;
    if (_endAt == -1)
        _endAt = _end;
    setCurrentFrame(start);
    _status = 0;
    _completeCallback = completeCallback;
}

void MovieClip::drawFrame()
{
    _forceDraw = false;
    _graphics->clearMesh();

    if (_currentFrame < _frameCount)
    {
        const Frame& frame = _frames[_currentFrame];

        if (frame.rect.GetSizeX() > 0)
        {
            VRectanglef uvRect = frame.uvRect;
            if (_flip != FlipType::NONE)
                ToolSet::flipRect(uvRect, _flip);

            _graphics->addQuad(frame.rect, uvRect, _color);
            if (frame.rotated)
                _graphics->rotateUV(uvRect);
        }
    }
}

void MovieClip::update(float dt)
{
    if (_playing && _frameCount != 0 && _status != 3)
    {
        _playState->update(this, dt);
        if (_forceDraw || _currentFrame != _playState->getCurrentFrame())
        {
            if (_status == 1)
            {
                _currentFrame = _start;
                _playState->setCurrentFrame(_currentFrame);
                _status = 0;
            }
            else if (_status == 2)
            {
                _currentFrame = _endAt;
                _playState->setCurrentFrame(_currentFrame);
                _status = 3;

                if (_completeCallback)
                    scheduleOnce(SCHEDULE_SELECTOR(MovieClip::playCompleted));
            }
            else
            {
                _currentFrame = _playState->getCurrentFrame();
                if (_currentFrame == _end)
                {
                    if (_times > 0)
                    {
                        _times--;
                        if (_times == 0)
                            _status = 2;
                        else
                            _status = 1;
                    }
                    else if (_start != 0)
                        _status = 1;
                }
            }
            drawFrame();
        }
    }
    else if (_forceDraw)
        drawFrame();

    Image::update(dt);
}

void MovieClip::playCompleted(float)
{
    if (_status == 3 && _completeCallback)
    {
        std::function<void()> cb = _completeCallback;
        _completeCallback = nullptr;
        cb();
    }
}

void MovieClip::rebuild()
{
    if (_frameCount > 0)
    {
        _requireUpdateMesh = false;
        drawFrame();
    }
    else
        Image::rebuild();
}

NS_FGUI_END
