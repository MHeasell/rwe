#ifndef RWE_VAOHANDLE_H
#define RWE_VAOHANDLE_H

#include <GL/glew.h>
#include <rwe/GlIdentifier.h>
#include <rwe/SharedHandle.h>

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

#endif
