#include "SharedTextureHandle.h"

namespace rwe
{
    SharedTextureHandle::SharedTextureHandle(GLuint handle) : handle(handle), referenceCount(new unsigned int(1)) {}

    SharedTextureHandle::~SharedTextureHandle()
    {
        destroy();
    }

    SharedTextureHandle::SharedTextureHandle(const SharedTextureHandle& other) : handle(other.handle), referenceCount(other.referenceCount)
    {
        if (isValid())
        {
            ++(*referenceCount);
        }
    }

    SharedTextureHandle& SharedTextureHandle::operator=(const SharedTextureHandle& other)
    {
        destroy();

        handle = other.handle;
        referenceCount = other.referenceCount;

        if (isValid())
        {
            ++(*referenceCount);
        }

        return *this;
    }

    SharedTextureHandle::SharedTextureHandle(SharedTextureHandle&& other) noexcept : handle(other.handle), referenceCount(other.referenceCount)
    {
        other.handle = 0;
        other.referenceCount = nullptr;
    }

    SharedTextureHandle& SharedTextureHandle::operator=(SharedTextureHandle&& other) noexcept
    {
        destroy();

        handle = other.handle;
        referenceCount = other.referenceCount;

        other.handle = 0;
        other.referenceCount = nullptr;

        return *this;
    }

    void SharedTextureHandle::destroy()
    {
        if (isValid())
        {
            if (--(*referenceCount) == 0)
            {
                glDeleteTextures(1, &handle);
                delete referenceCount;
            }
        }
    }

    bool SharedTextureHandle::isValid() const
    {
        return handle != 0;
    }

    SharedTextureHandle::operator bool() const
    {
        return isValid();
    }

    GLuint SharedTextureHandle::get() const
    {
        return handle;
    }

    void SharedTextureHandle::reset(GLuint newTexture)
    {
        destroy();

        if (newTexture == 0)
        {
            handle = 0;
            referenceCount = nullptr;
        }
        else
        {
            handle = newTexture;
            referenceCount = new unsigned int(1);
        }
    }

    void SharedTextureHandle::reset()
    {
        destroy();

        handle = 0;
        referenceCount = nullptr;
    }

    unsigned int SharedTextureHandle::useCount() const
    {
        if (referenceCount == nullptr)
        {
            return 0;
        }

        return *referenceCount;
    }

    SharedTextureHandle::SharedTextureHandle() : handle(0), referenceCount(nullptr) {}

    bool SharedTextureHandle::operator==(const SharedTextureHandle& rhs) const
    {
        return handle == rhs.handle;
    }

    bool SharedTextureHandle::operator!=(const SharedTextureHandle& rhs) const
    {
        return !(rhs == *this);
    }
}
