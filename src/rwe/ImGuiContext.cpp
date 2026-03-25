#include "ImGuiContext.h"
#include <SDL3/SDL_events.h>

namespace rwe
{
    ImGuiContext::~ImGuiContext()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    bool wantsEvent(const ImGuiIO& io, const SDL_Event& event)
    {
        if (io.WantCaptureKeyboard)
        {
            if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP || event.type == SDL_EVENT_TEXT_INPUT)
            {
                return true;
            }
        }

        if (io.WantCaptureMouse)
        {
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP || event.type == SDL_EVENT_MOUSE_MOTION || event.type == SDL_EVENT_MOUSE_WHEEL)
            {
                return true;
            }
        }

        return false;
    }

    bool ImGuiContext::processEvent(const SDL_Event& event)
    {
        if (wantsEvent(*io, event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            return true;
        }
        return false;
    }

    ImGuiContext::ImGuiContext(const std::string& iniPath, SDL_Window* window, void* glContext) : iniPath(iniPath)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        io = &ImGui::GetIO();
        io->IniFilename = this->iniPath.data();
        io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
        ImGui_ImplSDL3_InitForOpenGL(window, glContext);
        ImGui_ImplOpenGL3_Init("#version 150");
    }

    void ImGuiContext::newFrame(SDL_Window* window)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiContext::render()
    {
        ImGui::Render();
    }

    void ImGuiContext::renderDrawData()
    {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
