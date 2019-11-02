#include "ImGuiContext.h"
#include <SDL_events.h>

namespace rwe
{
    ImGuiContext::~ImGuiContext()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    bool wantsEvent(const ImGuiIO& io, const SDL_Event& event)
    {
        if (io.WantCaptureKeyboard)
        {
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                return true;
            }
        }

        if (io.WantCaptureMouse)
        {
            if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEWHEEL)
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
            ImGui_ImplSDL2_ProcessEvent(&event);
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
        ImGui_ImplSDL2_InitForOpenGL(window, glContext);
        ImGui_ImplOpenGL3_Init("#version 130");
    }

    void ImGuiContext::newFrame(SDL_Window* window)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
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
