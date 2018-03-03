#ifndef RWE_OPENGLVERSION_H
#define RWE_OPENGLVERSION_H

namespace rwe
{
    struct OpenGlVersion
    {
        int major;
        int minor;

        OpenGlVersion(int major, int minor) : major(major), minor(minor)
        {
        }

        bool operator==(const OpenGlVersion& rhs) const
        {
            return major == rhs.major && minor == rhs.minor;
        }

        bool operator!=(const OpenGlVersion& rhs) const
        {
            return !(rhs == *this);
        }

        bool operator<(const OpenGlVersion& rhs) const
        {
            return major < rhs.major || (major == rhs.major && minor < rhs.minor);
        }

        bool operator>(const OpenGlVersion& rhs) const
        {
            return rhs < *this;
        }

        bool operator<=(const OpenGlVersion& rhs) const
        {
            return !(rhs < *this);
        }

        bool operator>=(const OpenGlVersion& rhs) const
        {
            return !(*this < rhs);
        }
    };
}

#endif
