#pragma once

#include <GL/glew.h>
#include <rwe/render/GlIdentifier.h>
#include <rwe/util/SharedHandle.h>

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
