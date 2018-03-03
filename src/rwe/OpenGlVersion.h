#ifndef RWE_OPENGLVERSION_H
#define RWE_OPENGLVERSION_H

namespace rwe
{
    enum class OpenGlProfile
    {
        Core,
        Compatibility
    };

    const char* getOpenGlProfileName(OpenGlProfile profile)
    {
        switch (profile)
        {
            case OpenGlProfile::Core: return "core";
            case OpenGlProfile::Compatibility: return "compatibility";
        }

        throw std::logic_error("Unknown profile");
    }

    SDL_GLprofile getSdlProfileMask(OpenGlProfile profile)
    {
        switch (profile)
        {
            case OpenGlProfile::Core: return SDL_GL_CONTEXT_PROFILE_CORE;
            case OpenGlProfile::Compatibility: return SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
        }

        throw std::logic_error("Unknown profile");
    }

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

    struct OpenGlVersionInfo
    {
        OpenGlVersion version;
        OpenGlProfile profile;

        OpenGlVersionInfo(const OpenGlVersion& version, OpenGlProfile profile) : version(version), profile(profile)
        {
        }
        OpenGlVersionInfo(int major, int minor, OpenGlProfile profile) : version(major, minor), profile(profile)
        {
        }
    };
}

#endif
