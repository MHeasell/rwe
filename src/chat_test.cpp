#include <GL/glew.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <rwe/ImGuiContext.h>
#include <rwe/io/hpi/HpiArchive.h>
#include <rwe/render/GraphicsContext.h>
#include <rwe/render/OpenGlVersion.h>
#include <rwe/sdl/SdlContextManager.h>
#include <rwe/util/Result.h>
#include <rwe/util/match.h>
#include <spdlog/spdlog.h>
#include <string>

namespace rwe
{
    OpenGlVersion getOpenGlContextVersion()
    {
        int major;
        int minor;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        return OpenGlVersion(major, minor);
    }

    Result<SdlContext::GlContextUniquePtr, const char*> createOpenGlContext(SdlContext* sdlContext, SDL_Window* window, spdlog::logger& logger, const OpenGlVersionInfo& requiredVersion)
    {
        logger.info(
            "Requesting OpenGL version {0}.{1}, {2} profile",
            requiredVersion.version.majorVersion,
            requiredVersion.version.minorVersion,
            getOpenGlProfileName(requiredVersion.profile));

        if (sdlContext->glSetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, requiredVersion.version.majorVersion) != 0)
        {
            return Err(SDL_GetError());
        }
        if (sdlContext->glSetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, requiredVersion.version.minorVersion) != 0)
        {
            return Err(SDL_GetError());
        }
        if (sdlContext->glSetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, getSdlProfileMask(requiredVersion.profile)) != 0)
        {
            return Err(SDL_GetError());
        }
        if (sdlContext->glSetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG) != 0)
        {
            return Err(SDL_GetError());
        }

        auto glContext = sdlContext->glCreateContext(window);
        if (glContext == nullptr)
        {
            return Err(SDL_GetError());
        }

        auto contextVersion = getOpenGlContextVersion();
        if (contextVersion < requiredVersion.version)
        {
            static const char* errMessage = "Created OpenGL context did not meet version requirements";
            return Err(errMessage);
        }

        return Ok(std::move(glContext));
    };

    void doGlewInit()
    {
        // Must set glewExperimental to true, otherwise in glew <= 1.13.0 glewInit() will fail
        // and set glError to GL_INVALID_ENUM if called in an OpenGL core context.
        // GL_INVALID_ENUM may *still* be emitted anyway, but isn't critical.
        // Ubuntu 16.04 and earlier uses glew 1.13.0, Ubuntu 18.04 uses glew 2.0.0.
        // See: https://www.khronos.org/opengl/wiki/OpenGL_Loading_Library
        glewExperimental = GL_TRUE;
        if (auto result = glewInit(); result != GLEW_OK)
        {
            throw std::runtime_error(reinterpret_cast<const char*>(glewGetErrorString(result)));
        }

        if (auto error = glGetError(); error == GL_NO_ERROR)
        {
            // all good, continue on
        }
        else if (error == GL_INVALID_ENUM)
        {
            // Ignore, expected from glew even with glewExperimental = true.
            // It should have still worked anyway.

            // Check if there are any more errors.
            if (auto nextError = glGetError(); nextError != GL_NO_ERROR)
            {
                throw OpenGlException(error);
            }
        }
        else
        {
            throw OpenGlException(error);
        }
    }

    void runChat(spdlog::logger& logger)
    {
        logger.info("Initializing SDL");
        SdlContextManager sdlManager;
        auto sdlContext = sdlManager.getSdlContext();

        auto window = sdlContext->createWindow("RWE Chat", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
        if (window == nullptr)
        {
            throw std::runtime_error(SDL_GetError());
        }

        logger.info("Initializing OpenGL context");

        auto glContextResult = createOpenGlContext(sdlContext, window.get(), logger, OpenGlVersionInfo(3, 2, OpenGlProfile::Core));
        if (!glContextResult)
        {
            logger.error("Failed to create preferred OpenGL context: {0}", glContextResult.getErr());
            glContextResult = createOpenGlContext(sdlContext, window.get(), logger, OpenGlVersionInfo(3, 0, OpenGlProfile::Compatibility));
            if (!glContextResult)
            {
                throw std::runtime_error(glContextResult.getErr());
            }
        }

        auto glContext = std::move(*glContextResult);
        if (glContext == nullptr)
        {
            throw std::runtime_error(SDL_GetError());
        }

        doGlewInit();

        // log opengl context info
        logger.info("OpenGL version: {0}", glGetString(GL_VERSION));
        logger.info("OpenGL vendor: {0}", glGetString(GL_VENDOR));
        logger.info("OpenGL renderer: {0}", glGetString(GL_RENDERER));
        logger.info("OpenGL shading language version: {0}", glGetString(GL_SHADING_LANGUAGE_VERSION));
        logger.debug("OpenGL extensions:");
        int openGlExtensionCount;
        glGetIntegerv(GL_NUM_EXTENSIONS, &openGlExtensionCount);
        for (int i = 0; i < openGlExtensionCount; ++i)
        {
            logger.debug("  {0}", glGetStringi(GL_EXTENSIONS, i));
        }

        logger.info("Initializing Dear ImGui");
        ImGuiContext imGuiContext("imgui.ini", window.get(), glContext.get());

        auto graphics = GraphicsContext();


        SDL_Event event;
        std::array<char, 128> message;

        int selectedMessage = 0;
        const char* otherMessages[] = {"Armoured Fish: Hello there", "Someone Else: How you doing?", "Armoured Fish: Not bad..."};

        std::fill(message.begin(), message.end(), '\0');
        while (true)
        {
            while (sdlContext->pollEvent(&event))
            {
                if (imGuiContext.processEvent(event))
                {
                    continue;
                }

                if (event.type == SDL_QUIT)
                {
                    return;
                }
            }

            imGuiContext.newFrame(window.get());

            ImGui::Begin("RWE Chat");

            ImGui::ListBox("", &selectedMessage, otherMessages, 3, 10);

            ImGui::InputText("", message.data(), message.size());
            ImGui::SameLine();
            ImGui::Button("Send");

            ImGui::End();

            imGuiContext.render();

            graphics.clear();
            imGuiContext.renderDrawData();
            sdlContext->glSwapWindow(window.get());
        }
    }

}


int main(int argc, char* argv[])
{
    auto logger = spdlog::stderr_color_mt("stderr");

    rwe::runChat(*logger);


    return 0;
}
