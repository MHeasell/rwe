#ifndef RWE_SDLCONTEXTMANAGER_H
#define RWE_SDLCONTEXTMANAGER_H

#include <stdexcept>
#include <memory>
#include <SDL.h>
#include <SDL_mixer.h>

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

        struct RWopsDeleter
        {
            void operator()(SDL_RWops* rw) { SDL_RWclose(rw); }
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

        std::unique_ptr<SDL_RWops, RWopsDeleter> rwFromConstMem(const void* mem, int size)
        {
            return std::unique_ptr<SDL_RWops, RWopsDeleter>(SDL_RWFromConstMem(mem, size));
        };

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

        Uint32 getMouseState(int* x, int* y)
        {
            return SDL_GetMouseState(x, y);
        }

        int showCursor(int toggle)
        {
            return SDL_ShowCursor(toggle);
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

    public:
        struct MixChunkDeleter
        {
            void operator()(Mix_Chunk* chunk) { Mix_FreeChunk(chunk); }
        };

        std::unique_ptr<Mix_Chunk, MixChunkDeleter> loadWavRw(SDL_RWops* src)
        {
            return std::unique_ptr<Mix_Chunk, MixChunkDeleter>(Mix_LoadWAV_RW(src, 0));
        };

        void allocateChannels(int numChans)
        {
            Mix_AllocateChannels(numChans);
        }

        int playChannel(int channel, Mix_Chunk* chunk, int loops)
        {
            return Mix_PlayChannel(channel, chunk, loops);
        }

        void haltChannel(int channel)
        {
            Mix_HaltChannel(channel);
        }

        int volumeChunk(Mix_Chunk* chunk, int volume)
        {
            return Mix_VolumeChunk(chunk, volume);
        }
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
        SdlMixerContext* getSdlMixerContext();
        const SdlImageContext* getSdlImageContext() const;

    private:
        SdlContext sdlContext;
        SdlNetContext sdlNetContext;
        SdlMixerContext sdlMixerContext;
        SdlImageContext sdlImageContext;
    };
}


#endif
