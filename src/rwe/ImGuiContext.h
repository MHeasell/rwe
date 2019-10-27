#pragma once

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <string>

namespace rwe
{
    struct ImGuiContext
    {
        std::string iniPath;
        ImGuiIO* io;

        ImGuiContext(const std::string& iniPath, SDL_Window* window, void* glContext);
        ~ImGuiContext();

        /* Returns true if event was consumed */
        bool processEvent(const SDL_Event& event);

        void newFrame(SDL_Window* window);

        void render();
    };
}
