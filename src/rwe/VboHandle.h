#pragma once

#include <GL/glew.h>
#include <rwe/GlIdentifier.h>
#include <rwe/SharedHandle.h>

namespace rwe
{
    struct VboIdentifierTag;
    using VboIdentifier = GlIdentifier<VboIdentifierTag>;

    struct VboHandleDeleter
    {
        void operator()(VboIdentifier id)
        {
            if (id.isValid())
            {
                glDeleteBuffers(1, &(id.value));
            }
        }
    };

    using VboHandle = UniqueHandle<VboIdentifier, VboHandleDeleter>;
    using SharedVboHandle = SharedHandle<VboIdentifier, VboHandleDeleter>;
}
