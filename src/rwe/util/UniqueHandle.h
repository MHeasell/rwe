#pragma once

namespace rwe
{
    template <typename T, typename Deleter>
    class UniqueHandle
    {
    public:
        using Type = UniqueHandle<T, Deleter>;

    private:
        T value;

    public:
        UniqueHandle() = default;
        explicit UniqueHandle(T value) : value(value)
        {
        }

        UniqueHandle(const Type&) = delete;
        Type& operator=(const Type&) = delete;

        UniqueHandle(Type&& that) noexcept : value(that.release())
        {
        }

        Type& operator=(Type&& that) noexcept
        {
            destroy();
            value = that.release();
            return *this;
        }

        ~UniqueHandle()
        {
            destroy();
        }

        /** Returns the underlying handle. */
        T get() const
        {
            return value;
        }

        /** Replaces the contents of the handle with the given value. */
        void reset(T newValue)
        {
            destroy();
            value = newValue;
        }

        /** Resets the handle to the default value. */
        void reset()
        {
            destroy();
            value = T();
        }

        /** Releases the underlying resource from the responsibility of the handle. */
        T release()
        {
            T tmp = value;
            value = T();
            return tmp;
        }

    private:
        void destroy()
        {
            Deleter()(value);
        }
    };
}
