#pragma once

#include <functional>
#include <memory>
#include <rwe/observable/Subscription.h>

namespace rwe
{
    template <typename T>
    class Observable
    {
    public:
        using SubscriberCallback = std::function<void(T)>;
        virtual std::unique_ptr<Subscription> subscribe(const SubscriberCallback& onNext) = 0;
        virtual std::unique_ptr<Subscription> subscribe(SubscriberCallback&& onNext) = 0;
    };
}
