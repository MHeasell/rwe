#ifndef RWE_SUBJECT_H
#define RWE_SUBJECT_H

#include <functional>
#include <memory>
#include <rwe/observable/Observable.h>
#include <rwe/observable/Subscription.h>

namespace rwe
{
    template <typename T>
    class Subject : public Observable<T>
    {
    public:
        using SubscriberCallback = typename Observable<T>::SubscriberCallback;

    private:
        using SubscriberId = unsigned int;

        struct Subscriber
        {
            SubscriberId id;
            SubscriberCallback callback;
        };

        class ConcreteSubscription : public Subscription
        {
        private:
            Subject* observable;
            SubscriberId id;

        public:
            ConcreteSubscription(Subject<T>* observable, SubscriberId id)
                : observable(observable), id(id)
            {
            }

            ConcreteSubscription(const ConcreteSubscription&) = delete;
            ConcreteSubscription& operator=(const ConcreteSubscription&) = delete;

        protected:
            void unsubscribe() override
            {
                observable->unsubscribe(id);
            }
        };

    private:
        SubscriberId nextId{0};

        std::vector<Subscriber> subscribers;

        void unsubscribe(SubscriberId id)
        {
            auto it = std::find_if(subscribers.begin(), subscribers.end(), [id](const Subscriber& s) { return s.id == id; });
            if (it == subscribers.end())
            {
                return;
            }

            subscribers.erase(it);
        }

    public:
        void next(const T& newValue)
        {
            for (const Subscriber& s : subscribers)
            {
                s.callback(newValue);
            }
        }

        std::unique_ptr<Subscription> subscribe(const SubscriberCallback& onNext) override
        {
            auto id = nextId++;
            subscribers.push_back({id, onNext});

            return std::unique_ptr<Subscription>(new ConcreteSubscription(this, id));
        }

        std::unique_ptr<Subscription> subscribe(SubscriberCallback&& onNext) override
        {
            auto id = nextId++;
            subscribers.push_back({id, std::move(onNext)});

            return std::unique_ptr<Subscription>(new ConcreteSubscription(this, id));
        }
    };
}

#endif
