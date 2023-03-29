#pragma once

#include <GL/glew.h>
#include <rwe/render/GlIdentifier.h>
#include <rwe/util/SharedHandle.h>

namespace rwe
{
    struct RenderBufferIdTag;
    using RenderBufferIdentifier = GlIdentifier<RenderBufferIdTag>;

    struct RenderBufferHandleDeleter
    {
        void operator()(RenderBufferIdentifier handle)
        {
            if (handle.isValid())
            {
                glDeleteRenderbuffers(1, &(handle.value));
            }
        }
    };

    using RenderBufferHandle = UniqueHandle<RenderBufferIdentifier, RenderBufferHandleDeleter>;
    using SharedRenderBufferHandle = SharedHandle<RenderBufferIdentifier, RenderBufferHandleDeleter>;
}
