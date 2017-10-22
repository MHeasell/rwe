#ifndef RWE_SHADERHANDLE_H
#define RWE_SHADERHANDLE_H

#include <functional>
#include <GL/glew.h>
#include <rwe/SharedHandle.h>

namespace rwe
{
    struct ShaderIdentifier
    {
        using ValueType = GLuint;

        ValueType value{0};

        ShaderIdentifier() = default;
        explicit ShaderIdentifier(ValueType value) : value(value)
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

        bool operator==(const ShaderIdentifier& rhs) const
        {
            return value == rhs.value;
        }

        bool operator!=(const ShaderIdentifier& rhs) const
        {
            return !(rhs == *this);
        }
    };
}

namespace std
{
    template <>
    struct hash<rwe::ShaderIdentifier>
    {
        std::size_t operator()(const rwe::ShaderIdentifier& f) const noexcept
        {
            return std::hash<rwe::ShaderIdentifier::ValueType>()(f.value);
        }
    };
}

namespace rwe
{
    struct ShaderHandleDeleter
    {
        void operator()(ShaderIdentifier id)
        {
            if (id.isValid())
            {
                glDeleteShader(id.value);
            }
        }
    };

    using ShaderHandle = UniqueHandle<ShaderIdentifier, ShaderHandleDeleter>;
    using SharedShaderHandle = SharedHandle<ShaderIdentifier, ShaderHandleDeleter>;
}

#endif
