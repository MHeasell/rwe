#ifndef RWE_SDLCONTEXTMANAGER_H
#define RWE_SDLCONTEXTMANAGER_H

#include <stdexcept>
#include <memory>
#include <SDL.h>

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
    public:
        struct WindowDeleter
        {
            void operator()(SDL_Window* window) { SDL_DestroyWindow(window); }
        };

        struct GlContextDeleter
        {
            void operator()(SDL_GLContext context) { SDL_GL_DeleteContext(context); }
        };

    private:
        SdlContext();
        SdlContext(const SdlContext&) = delete;
        ~SdlContext();

    public:
        std::unique_ptr<SDL_Window, WindowDeleter> createWindow(const char* title, int x, int y, int w, int h, Uint32 flags)
        {
            return std::unique_ptr<SDL_Window, WindowDeleter>(SDL_CreateWindow(title, x, y, w, h, flags));
        }

        std::unique_ptr<void, GlContextDeleter> glCreateContext(SDL_Window* window)
        {
            return std::unique_ptr<void, GlContextDeleter>(SDL_GL_CreateContext(window));
        }

        Uint32 getTicks()
        {
            return SDL_GetTicks();
        }

        void delay(Uint32 ms)
        {
            SDL_Delay(ms);
        }

        void glSwapWindow(SDL_Window* window)
        {
            SDL_GL_SwapWindow(window);
        }

        bool pollEvent(SDL_Event* event)
        {
            return SDL_PollEvent(event) == 1;
        }

    private:
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
        SdlContext* getSdlContext();
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
