#pragma once

namespace rwe
{
    class Subscription
    {
    public:
        virtual ~Subscription() = default;
        virtual void unsubscribe() = 0;
    };
}
