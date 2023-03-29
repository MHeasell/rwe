#pragma once

#include <variant>

namespace rwe
{
    // Result is implemented in this convoluted way
    // because MSVC does not yet support template argument deduction
    // for classes/constructors.
    // Putting the types in this namespace
    // and providing functions that look like the constructors
    // simulates the interface that callers would have had.
    namespace result
    {
        template <typename T>
        struct Ok
        {
            T value;

            constexpr Ok(const T& value) : value(value) {}
            constexpr Ok(T&& value) : value(std::move(value)) {}
        };

        template <typename T>
        struct Err
        {
            T value;

            constexpr Err(const T& value) : value(value) {}
            constexpr Err(T&& value) : value(std::move(value)) {}
        };
    }

    template <typename U>
    constexpr result::Ok<std::decay_t<U>> Ok(U&& value)
    {
        return result::Ok<std::decay_t<U>>(std::forward<U>(value));
    }

    template <typename U>
    constexpr result::Err<typename std::decay_t<U>> Err(U&& value)
    {
        return result::Err<std::decay_t<U>>(std::forward<U>(value));
    }

    template <typename T, typename E>
    class Result
    {
    private:
        std::variant<result::Ok<T>, result::Err<E>> value;

    public:
        Result(const result::Ok<T>& ok) : value(ok) {}
        Result(result::Ok<T>&& ok) : value(std::move(ok)) {}

        Result(const result::Err<E>& err) : value(err) {}
        Result(result::Err<E>&& err) : value(std::move(err)) {}

        bool isOk() const
        {
            return std::get_if<result::Ok<T>>(&value) != nullptr;
        }

        bool isErr() const
        {
            return std::get_if<result::Err<E>>(&value) != nullptr;
        }

        const T& get() const
        {
            return std::get<result::Ok<T>>(value).value;
        }

        T& get()
        {
            return std::get<result::Ok<T>>(value).value;
        }

        const E& getErr() const
        {
            return std::get<result::Err<E>>(value).value;
        }

        E& getErr()
        {
            return std::get<result::Err<E>>(value).value;
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
