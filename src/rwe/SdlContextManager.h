#ifndef RWE_SDLCONTEXTMANAGER_H
#define RWE_SDLCONTEXTMANAGER_H

#include <stdexcept>
#include <memory>

namespace rwe
{
    class SDLException : public std::runtime_error
    {
    public:
        explicit SDLException(const char* sdlError);
    };

    class SDLNetException : public std::runtime_error
    {
    public:
        explicit SDLNetException(const char* sdlError);
    };

    class SDLMixerException : public std::runtime_error
    {
    public:
        explicit SDLMixerException(const char* sdlError);
    };

    class SDLImageException : public std::runtime_error
    {
    public:
        explicit SDLImageException(const char* sdlError);
    };

    class SdlContext
    {
    private:
        SdlContext();
        SdlContext(const SdlContext&) = delete;
        ~SdlContext();

        friend class SdlContextManager;
    };

    class SdlNetContext
    {
    private:
        SdlNetContext();
        SdlNetContext(const SdlNetContext&) = delete;
        ~SdlNetContext();

        friend class SdlContextManager;
    };

    class SdlMixerContext
    {
    private:
        SdlMixerContext();
        SdlMixerContext(const SdlMixerContext&) = delete;
        ~SdlMixerContext();

        friend class SdlContextManager;
    };

    class SdlImageContext
    {
    private:
        SdlImageContext();
        SdlImageContext(const SdlImageContext&) = delete;
        ~SdlImageContext();

        friend class SdlContextManager;
    };

    /**
	 * Manages the lifetime of SDL contexts.
	 * Contexts accessed via this manager
	 * are only valid for the lifetime of the manager.
	 *
	 * SDL is a global resource so don't instantiate more than one of these.
	 */
    class SdlContextManager
    {
    public:
        SdlContextManager();
        SdlContextManager(const SdlContextManager&) = delete;

        const SdlContext* getSdlContext() const;
        const SdlNetContext* getSdlNetContext() const;
        const SdlMixerContext* getSdlMixerContext() const;
        const SdlImageContext* getSdlImageContext() const;

    private:
        SdlContext sdlContext;
        SdlNetContext sdlNetContext;
        SdlMixerContext sdlMixerContext;
        SdlImageContext sdlImageContext;
    };
}


#endif
