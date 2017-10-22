#ifndef RWE_VBOHANDLE_H
#define RWE_VBOHANDLE_H

#include <functional>
#include <GL/glew.h>
#include <rwe/SharedHandle.h>

namespace rwe
{
    struct VboIdentifier
    {
        using ValueType = GLuint;

        ValueType value{0};

        VboIdentifier() = default;
        explicit VboIdentifier(ValueType value) : value(value)
        {
        }

        bool isValid() const
        {
            return value != 0;
        }

        explicit operator bool() const
        {
            return isValid();
        }

        bool operator==(const VboIdentifier& rhs) const
        {
            return value == rhs.value;
        }

        bool operator!=(const VboIdentifier& rhs) const
        {
            return !(rhs == *this);
        }
    };
}

namespace std
{
    template <>
    struct hash<rwe::VboIdentifier>
    {
        std::size_t operator()(const rwe::VboIdentifier& f) const noexcept
        {
            return std::hash<rwe::VboIdentifier::ValueType>()(f.value);
        }
    };
}

namespace rwe
{
    struct VboHandleDeleter
    {
        void operator()(VboIdentifier id)
        {
            if (id.isValid())
            {
                glDeleteBuffers(1, &(id.value));
            }
        }
    };

    using VboHandle = UniqueHandle<VboIdentifier, VboHandleDeleter>;
    using SharedVboHandle = SharedHandle<VboIdentifier, VboHandleDeleter>;
}

#endif
