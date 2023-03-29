#pragma once

#include <GL/glew.h>
#include <rwe/render/GlIdentifier.h>
#include <rwe/util/SharedHandle.h>

namespace rwe
{
    struct ShaderProgramIdentifierTag;
    using ShaderProgramIdentifier = GlIdentifier<ShaderProgramIdentifierTag>;

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
