#ifndef RWE_SHADERPROGRAMHANDLE_H
#define RWE_SHADERPROGRAMHANDLE_H

#include <functional>
#include <GL/glew.h>
#include <rwe/SharedHandle.h>

namespace rwe
{
    struct ShaderProgramIdentifier
    {
        using ValueType = GLuint;

        ValueType value{0};

        ShaderProgramIdentifier() = default;
        explicit ShaderProgramIdentifier(ValueType value) : value(value)
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

        bool operator==(const ShaderProgramIdentifier& rhs) const
        {
            return value == rhs.value;
        }

        bool operator!=(const ShaderProgramIdentifier& rhs) const
        {
            return !(rhs == *this);
        }
    };
}

namespace std
{
    template <>
    struct hash<rwe::ShaderProgramIdentifier>
    {
        std::size_t operator()(const rwe::ShaderProgramIdentifier& f) const noexcept
        {
            return std::hash<rwe::ShaderProgramIdentifier::ValueType>()(f.value);
        }
    };
}

namespace rwe
{
    struct ShaderProgramHandleDeleter
    {
        void operator()(ShaderProgramIdentifier id)
        {
            if (id.isValid())
            {
                glDeleteShader(id.value);
            }
        }
    };

    using ShaderProgramHandle = UniqueHandle<ShaderProgramIdentifier, ShaderProgramHandleDeleter>;
    using SharedShaderProgramHandle = SharedHandle<ShaderProgramIdentifier, ShaderProgramHandleDeleter>;
}

#endif
