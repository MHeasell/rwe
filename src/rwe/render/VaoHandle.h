#pragma once

#include <GL/glew.h>
#include <rwe/render/GlIdentifier.h>
#include <rwe/util/SharedHandle.h>

namespace rwe
{
    struct VaoIdentifierTag;
    using VaoIdentifier = GlIdentifier<VaoIdentifierTag>;

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
