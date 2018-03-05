#ifndef RWE_BEHAVIORSUBJECT_H
#define RWE_BEHAVIORSUBJECT_H

#include <functional>
#include <rwe/observable/Subject.h>
#include <rwe/observable/Subscription.h>

namespace rwe
{
    template <typename T>
    class BehaviorSubject : public Observable<T>
    {
    public:
        using SubscriberCallback = typename Subject<T>::SubscriberCallback;

    private:
        T value;
        Subject<T> subject;

    public:
        BehaviorSubject() = default;
        BehaviorSubject(const T& value) : value(value) {}
        BehaviorSubject(T&& value) : value(std::move(value)) {}

        void next(const T& newValue)
        {
            if (newValue != value)
            {
                value = newValue;
                subject.next(value);
            }
        }
        void next(T&& newValue)
        {
            if (newValue != value)
            {
                value = std::move(newValue);
                subject.next(value);
            }
        }

        const T& getValue() const
        {
            return value;
        }

        std::unique_ptr<Subscription> subscribe(const SubscriberCallback& onNext) override
        {
            onNext(value);
            return subject.subscribe(onNext);
        }

        std::unique_ptr<Subscription> subscribe(SubscriberCallback&& onNext) override
        {
            onNext(value);
            return subject.subscribe(std::move(onNext));
        }
    };
}

#endif
