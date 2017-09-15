#ifndef RWE_BEHAVIORSUBJECT_H
#define RWE_BEHAVIORSUBJECT_H

#include <functional>
#include <rwe/observable/Subscription.h>
#include "Subject.h"

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
        BehaviorSubject(const T& value);
        BehaviorSubject(T&& value);

        void next(const T& newValue);
        void next(T&& newValue);

        const T& getValue() const;

        std::unique_ptr<Subscription> subscribe(const SubscriberCallback& onNext) override;
        std::unique_ptr<Subscription> subscribe(SubscriberCallback&& onNext) override;
    };

    template <typename T>
    BehaviorSubject<T>::BehaviorSubject(const T& value) : value(value) {}

    template <typename T>
    BehaviorSubject<T>::BehaviorSubject(T&& value) : value(std::move(value)) {}


    template <typename T>
    void BehaviorSubject<T>::next(const T& newValue)
    {
        if (newValue != value)
        {
            value = newValue;
            subject.next(value);
        }
    }

    template <typename T>
    void BehaviorSubject<T>::next(T&& newValue)
    {
        if (newValue != value)
        {
            value = std::move(newValue);
            subject.next(value);
        }
    }

    template <typename T>
    const T& BehaviorSubject<T>::getValue() const
    {
        return value;
    }

    template <typename T>
    std::unique_ptr<Subscription> BehaviorSubject<T>::subscribe(const BehaviorSubject<T>::SubscriberCallback& onNext)
    {
        onNext(value);
        return subject.subscribe(onNext);
    }

    template <typename T>
    std::unique_ptr<Subscription> BehaviorSubject<T>::subscribe(BehaviorSubject<T>::SubscriberCallback&& onNext)
    {
        onNext(value);
        return subject.subscribe(std::move(onNext));
    }
}

#endif
