#pragma once

#include <GL/glew.h>
#include <functional>
#include <rwe/util/OpaqueId.h>

namespace rwe
{
    template <typename Tag>
    struct GlIdentifier : public OpaqueId<GLuint, Tag>
    {
        GlIdentifier() : OpaqueId<GLuint, Tag>(0) {}
        explicit GlIdentifier(GLuint value) : OpaqueId<GLuint, Tag>(value)
        {
        }

        bool isValid() const
        {
            return this->value != 0;
        }

        explicit operator bool() const
        {
            return isValid();
        }
    };
}

namespace std
{
    template <typename Tag>
    struct hash<rwe::GlIdentifier<Tag>>
    {
        std::size_t operator()(const rwe::GlIdentifier<Tag>& f) const noexcept
        {
            return std::hash<typename rwe::GlIdentifier<Tag>::ValueType>()(f.value);
        }
    };
}
