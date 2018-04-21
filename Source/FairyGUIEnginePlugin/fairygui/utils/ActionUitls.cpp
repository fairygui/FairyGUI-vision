#include "ActionUtils.h"

NS_FGUI_BEGIN

ActionInterval * ActionUtils::createEaseAction(tweenfunc::TweenType tweenType, ActionInterval * action)
{
    switch (tweenType)
    {
    case tweenfunc::Linear:
        return EaseIn::create(action, 1);
    case tweenfunc::Elastic_EaseIn:
        return EaseElasticIn::create(action);
    case tweenfunc::Elastic_EaseOut:
        return EaseElasticOut::create(action);
    case tweenfunc::Elastic_EaseInOut:
        return EaseElasticInOut::create(action);

    case tweenfunc::Quad_EaseIn:
        return EaseQuadraticActionIn::create(action);
    case tweenfunc::Quad_EaseOut:
        return EaseQuadraticActionOut::create(action);
    case tweenfunc::Quad_EaseInOut:
        return EaseQuadraticActionInOut::create(action);

    case tweenfunc::Cubic_EaseIn:
        return EaseCubicActionIn::create(action);
    case tweenfunc::Cubic_EaseOut:
        return EaseCubicActionOut::create(action);
    case tweenfunc::Cubic_EaseInOut:
        return EaseCubicActionInOut::create(action);

    case tweenfunc::Quart_EaseIn:
        return EaseQuarticActionIn::create(action);
    case tweenfunc::Quart_EaseOut:
        return EaseQuarticActionOut::create(action);
    case tweenfunc::Quart_EaseInOut:
        return EaseQuarticActionInOut::create(action);

    case tweenfunc::Sine_EaseIn:
        return EaseSineIn::create(action);
    case tweenfunc::Sine_EaseOut:
        return EaseSineOut::create(action);
    case tweenfunc::Sine_EaseInOut:
        return EaseSineInOut::create(action);

    case tweenfunc::Bounce_EaseIn:
        return EaseBounceIn::create(action);
    case tweenfunc::Bounce_EaseOut:
        return EaseBounceOut::create(action);
    case tweenfunc::Bounce_EaseInOut:
        return EaseBounceInOut::create(action);

    case tweenfunc::Circ_EaseIn:
        return EaseCircleActionIn::create(action);
    case tweenfunc::Circ_EaseOut:
        return EaseCircleActionOut::create(action);
    case tweenfunc::Circ_EaseInOut:
        return EaseCircleActionInOut::create(action);

    case tweenfunc::Expo_EaseIn:
        return EaseExponentialIn::create(action);
    case tweenfunc::Expo_EaseOut:
        return EaseExponentialOut::create(action);
    case tweenfunc::Expo_EaseInOut:
        return EaseExponentialInOut::create(action);

    case tweenfunc::Back_EaseIn:
        return EaseBackIn::create(action);
    case tweenfunc::Back_EaseOut:
        return EaseBackOut::create(action);
    case tweenfunc::Back_EaseInOut:
        return EaseBackInOut::create(action);

    default:
        return EaseExponentialOut::create(action);
    }
}

ActionInterval* ActionUtils::composeActions(ActionInterval* mainAction, tweenfunc::TweenType easeType, float delay, std::function<void()> func, int tag)
{
    ActionInterval* action = createEaseAction(easeType, mainAction);
    if (func != nullptr)
    {
        FiniteTimeAction* callbackAction = CallFunc::create(func);
        if (delay > 0)
            action = Sequence::create(DelayTime::create(delay), action, callbackAction, NULL);
        else
            action = Sequence::createWithTwoActions(action, callbackAction);
    }
    else
    {
        if (delay > 0)
            action = Sequence::createWithTwoActions(DelayTime::create(delay), action);
    }
    action->setTag(tag);

    return action;
}

NS_FGUI_END
