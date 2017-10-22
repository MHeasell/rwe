#ifndef RWE_VAOHANDLE_H
#define RWE_VAOHANDLE_H

#include <functional>
#include <GL/glew.h>
#include <rwe/SharedHandle.h>

namespace rwe
{
    struct VaoIdentifier
    {
        using ValueType = GLuint;

        ValueType value{0};

        VaoIdentifier() = default;
        explicit VaoIdentifier(ValueType value) : value(value)
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

        bool operator==(const VaoIdentifier& rhs) const
        {
            return value == rhs.value;
        }

        bool operator!=(const VaoIdentifier& rhs) const
        {
            return !(rhs == *this);
        }
    };
}

namespace std
{
    template <>
    struct hash<rwe::VaoIdentifier>
    {
        std::size_t operator()(const rwe::VaoIdentifier& f) const noexcept
        {
            return std::hash<rwe::VaoIdentifier::ValueType>()(f.value);
        }
    };
}

namespace rwe
{
    struct VaoHandleDeleter
    {
        void operator()(VaoIdentifier id)
        {
            if (id.isValid())
            {
                glDeleteVertexArrays(1, &(id.value));
            }
        }
    };

    using VaoHandle = UniqueHandle<VaoIdentifier, VaoHandleDeleter>;
    using SharedVaoHandle = SharedHandle<VaoIdentifier, VaoHandleDeleter>;
}

#endif
