#ifndef RWE_SUBSCRIPTION_H
#define RWE_SUBSCRIPTION_H

namespace rwe
{
    class Subscription
    {
    public:
        virtual ~Subscription() = default;
    protected:
        virtual void unsubscribe() = 0;
    };
}

#endif
