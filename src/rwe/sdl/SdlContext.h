#pragma once

#include <SDL.h>
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
            void operator()(SDL_GLContext context) { SDL_GL_DeleteContext(context); }
        };

        struct RWopsDeleter
        {
            void operator()(SDL_RWops* rw) { SDL_RWclose(rw); }
        };

        using GlContextUniquePtr = std::unique_ptr<void, GlContextDeleter>;

    private:
        SdlContext()
        {
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
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
        std::unique_ptr<SDL_Window, WindowDeleter> createWindow(const char* title, int x, int y, int w, int h, Uint32 flags)
        {
            return std::unique_ptr<SDL_Window, WindowDeleter>(SDL_CreateWindow(title, x, y, w, h, flags));
        }

        GlContextUniquePtr glCreateContext(SDL_Window* window)
        {
            return GlContextUniquePtr(SDL_GL_CreateContext(window));
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

        int glSetAttribute(SDL_GLattr attr, int value)
        {
            return SDL_GL_SetAttribute(attr, value);
        }

        int glSetSwapInterval(int interval)
        {
            return SDL_GL_SetSwapInterval(interval);
        }

        void getWindowSize(SDL_Window* window, int* w, int* h)
        {
            SDL_GetWindowSize(window, w, h);
        }

        int getWindowDisplayIndex(SDL_Window* window)
        {
            return SDL_GetWindowDisplayIndex(window);
        }

        SDL_DisplayMode* getClosestDisplayMode(int displayIndex, const SDL_DisplayMode* mode, SDL_DisplayMode* closest)
        {
            return SDL_GetClosestDisplayMode(displayIndex, mode, closest);
        }

        int setWindowDisplayMode(SDL_Window* window, const SDL_DisplayMode* mode)
        {
            return SDL_SetWindowDisplayMode(window, mode);
        }

        void setWindowSize(SDL_Window* window, int width, int height)
        {
            return SDL_SetWindowSize(window, width, height);
        }

        void setWindowGrab(SDL_Window* window, SDL_bool grabbed)
        {
            SDL_SetWindowGrab(window, grabbed);
        }

        Uint32 getWindowId(SDL_Window* window)
        {
            return SDL_GetWindowID(window);
        }

    private:
        friend class SdlContextManager;
    };
}
