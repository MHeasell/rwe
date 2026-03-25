#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <rwe/sdl/SdlException.h>

namespace rwe
{
    class SdlContext
    {
    public:
        struct WindowDeleter
        {
            void operator()(SDL_Window* window) { SDL_DestroyWindow(window); }
        };

        struct GlContextDeleter
        {
            void operator()(SDL_GLContextState* context) { SDL_GL_DestroyContext(context); }
        };

        struct RWopsDeleter
        {
            void operator()(SDL_IOStream* rw) { SDL_CloseIO(rw); }
        };

        using GlContextUniquePtr = std::unique_ptr<SDL_GLContextState, GlContextDeleter>;

    private:
        SdlContext()
        {
            if (!SDL_Init(SDL_INIT_VIDEO)) // TODO: re-add SDL_INIT_AUDIO after SDL3_mixer migration
            {
                throw rwe::SDLException(SDL_GetError());
            }
        }
        SdlContext(const SdlContext&) = delete;
        ~SdlContext()
        {
            SDL_Quit();
        }

    public:
        std::unique_ptr<SDL_Window, WindowDeleter> createWindow(const char* title, int w, int h, SDL_WindowFlags flags)
        {
            return std::unique_ptr<SDL_Window, WindowDeleter>(SDL_CreateWindow(title, w, h, flags));
        }

        GlContextUniquePtr glCreateContext(SDL_Window* window)
        {
            return GlContextUniquePtr(SDL_GL_CreateContext(window));
        }

        std::unique_ptr<SDL_IOStream, RWopsDeleter> rwFromConstMem(const void* mem, size_t size)
        {
            return std::unique_ptr<SDL_IOStream, RWopsDeleter>(SDL_IOFromConstMem(mem, size));
        };

        Uint64 getTicks()
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
            return SDL_PollEvent(event);
        }

        SDL_MouseButtonFlags getMouseState(float* x, float* y)
        {
            return SDL_GetMouseState(x, y);
        }

        void showCursor()
        {
            SDL_ShowCursor();
        }

        void hideCursor()
        {
            SDL_HideCursor();
        }

        bool glSetAttribute(SDL_GLAttr attr, int value)
        {
            return SDL_GL_SetAttribute(attr, value);
        }

        bool glSetSwapInterval(int interval)
        {
            return SDL_GL_SetSwapInterval(interval);
        }

        void getWindowSize(SDL_Window* window, int* w, int* h)
        {
            SDL_GetWindowSize(window, w, h);
        }

        SDL_DisplayID getWindowDisplayIndex(SDL_Window* window)
        {
            return SDL_GetDisplayForWindow(window);
        }

        bool getClosestDisplayMode(SDL_DisplayID displayID, int w, int h, float refreshRate, SDL_DisplayMode* closest)
        {
            return SDL_GetClosestFullscreenDisplayMode(displayID, w, h, refreshRate, false, closest);
        }

        bool setWindowDisplayMode(SDL_Window* window, const SDL_DisplayMode* mode)
        {
            return SDL_SetWindowFullscreenMode(window, mode);
        }

        bool setWindowSize(SDL_Window* window, int width, int height)
        {
            return SDL_SetWindowSize(window, width, height);
        }

        void setWindowGrab(SDL_Window* window, bool grabbed)
        {
            SDL_SetWindowMouseGrab(window, grabbed);
        }

        SDL_WindowID getWindowId(SDL_Window* window)
        {
            return SDL_GetWindowID(window);
        }

    private:
        friend class SdlContextManager;
    };
}
