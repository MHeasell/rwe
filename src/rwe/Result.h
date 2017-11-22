#ifndef RWE_RESULT_H
#define RWE_RESULT_H

#include <boost/variant.hpp>

namespace rwe
{
    template <typename T>
    struct Ok
    {
        T value;

        Ok(const T& value) : value(value) {}
        Ok(T&& value) : value(std::move(value)) {}
    };

    template <typename E>
    struct Err
    {
        E value;

        Err(const E& value) : value(value) {}
        Err(E&& value) : value(std::move(value)) {}
    };

    template <typename T, typename E>
    class Result
    {
    private:
        boost::variant<Ok<T>, Err<E>> value;

    public:
        Result(const Ok<T>& ok) : value(ok) {}
        Result(Ok<T>&& ok) : value(std::move(ok)) {}

        Result(const Err<E>& err) : value(err) {}
        Result(Err<E>&& err) : value(std::move(err)) {}

        bool isOk() const
        {
            return boost::get<Ok<T>>(&value) != nullptr;
        }

        bool isErr() const
        {
            return boost::get<Err<E>>(&value) != nullptr;
        }

        const T& get() const
        {
            return boost::get<Ok<T>>(value).value;
        }

        T& get()
        {
            return boost::get<Ok<T>>(value).value;
        }

        const E& getErr() const
        {
            return boost::get<Err<E>>(value).value;
        }

        E& getErr()
        {
            return boost::get<Err<E>>(value).value;
        }

        operator bool() const
        {
            return isOk();
        }

        const T& operator*() const
        {
            return get();
        }

        T& operator*()
        {
            return get();
        }
    };
}

#endif
