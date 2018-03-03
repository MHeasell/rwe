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
        int majorVersion;
        int minorVersion;

        OpenGlVersion(int majorVersion, int minorVersion) : majorVersion(majorVersion), minorVersion(minorVersion)
        {
        }

        bool operator==(const OpenGlVersion& rhs) const
        {
            return majorVersion == rhs.majorVersion && minorVersion == rhs.minorVersion;
        }

        bool operator!=(const OpenGlVersion& rhs) const
        {
            return !(rhs == *this);
        }

        bool operator<(const OpenGlVersion& rhs) const
        {
            return majorVersion < rhs.majorVersion || (majorVersion == rhs.majorVersion && minorVersion < rhs.minorVersion);
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
        OpenGlVersionInfo(int majorVersion, int minorVersion, OpenGlProfile profile) : version(majorVersion, minorVersion), profile(profile)
        {
        }
    };
}

#endif
