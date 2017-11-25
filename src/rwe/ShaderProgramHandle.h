#ifndef RWE_SHADERPROGRAMHANDLE_H
#define RWE_SHADERPROGRAMHANDLE_H

#include <GL/glew.h>
#include <rwe/GlIdentifier.h>
#include <rwe/SharedHandle.h>

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

#endif
