#pragma once

#include <GL/glew.h>
#include <rwe/GlIdentifier.h>
#include <rwe/SharedHandle.h>

namespace rwe
{
    struct ShaderIdentifierTag;
    using ShaderIdentifier = GlIdentifier<ShaderIdentifierTag>;

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
