#pragma once

#include <GL/glew.h>
#include <rwe/SharedHandle.h>
#include <rwe/render/GlIdentifier.h>

namespace rwe
{
    struct FrameBufferIdTag;
    using FrameBufferIdentifier = GlIdentifier<FrameBufferIdTag>;

    struct FrameBufferHandleDeleter
    {
        void operator()(FrameBufferIdentifier handle)
        {
            if (handle.isValid())
            {
                glDeleteFramebuffers(1, &(handle.value));
            }
        }
    };

    using FrameBufferHandle = UniqueHandle<FrameBufferIdentifier, FrameBufferHandleDeleter>;
    using SharedFrameBufferHandle = SharedHandle<FrameBufferIdentifier, FrameBufferHandleDeleter>;
}
