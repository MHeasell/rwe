#ifndef RWE_SHAREDHANDLE_H
#define RWE_SHAREDHANDLE_H

#include <cassert>
#include <rwe/UniqueHandle.h>

namespace rwe
{
    template <typename Value, typename Deleter>
    class SharedHandle
    {
    public:
        using Type = SharedHandle<Value, Deleter>;

    private:
        Value handle;
        unsigned int* referenceCount;

    public:
        SharedHandle() : handle(), referenceCount(nullptr) {}

        explicit SharedHandle(Value handle) : handle(handle), referenceCount(new unsigned int(1)) {}

        ~SharedHandle()
        {
            destroy();
        }

        SharedHandle(const Type& other) : handle(other.handle), referenceCount(other.referenceCount)
        {
            if (isValid())
            {
                ++(*referenceCount);
            }
        }

        SharedHandle& operator=(const Type& other)
        {
            destroy();

            handle = other.handle;
            referenceCount = other.referenceCount;

            if (isValid())
            {
                ++(*referenceCount);
            }

            return *this;
        }

        SharedHandle(Type&& other) noexcept : handle(other.handle), referenceCount(other.referenceCount)
        {
            other.referenceCount = nullptr;
        }

        SharedHandle& operator=(Type&& other) noexcept
        {
            destroy();

            handle = other.handle;
            referenceCount = other.referenceCount;

            other.referenceCount = nullptr;

            return *this;
        }

        explicit SharedHandle(UniqueHandle<Value, Deleter>&& other) noexcept : SharedHandle(other.release())
        {
        }

        bool operator==(const Type& rhs) const
        {
            return referenceCount == rhs.referenceCount;
        }

        bool operator!=(const Type& rhs) const
        {
            return !(rhs == *this);
        }

        /**
         * Equivalent to isValid()
         */
        explicit operator bool() const
        {
            return isValid();
        }

        /** Returns the underlying texture handle. */
        Value get() const
        {
            assert(isValid());
            return handle;
        }

        /**
         * Returns true if the handle contains a valid resource, otherwise false.
         */
        bool isValid() const
        {
            return referenceCount != nullptr;
        }

        unsigned int useCount() const
        {
            if (referenceCount == nullptr)
            {
                return 0;
            }

            return *referenceCount;
        }

        /** Replaces the contents of the handle with the given value. */
        void reset(Value newValue)
        {
            destroy();
            handle = newValue;
            referenceCount = new unsigned int(1);
        }

        /** Resets the handle to the empty state. */
        void reset()
        {
            destroy();
            referenceCount = nullptr;
        }

    private:
        void destroy()
        {
            if (isValid())
            {
                if (--(*referenceCount) == 0)
                {
                    Deleter()(handle);
                    delete referenceCount;
                }
            }
        }
    };
}

#endif
