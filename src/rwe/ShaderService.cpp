#include "ShaderService.h"
#include <fstream>
#include <sstream>

namespace rwe
{
    ShaderService ShaderService::createShaderService(GraphicsContext& graphics)
    {
        ShaderService s;

        std::vector<AttribMapping> texturedVertexAttribs{
            AttribMapping{"position", 0},
            AttribMapping{"texCoord", 1}};

        std::vector<AttribMapping> coloredVertexAttribs{
            AttribMapping{"position", 0},
            AttribMapping{"color", 1}};

        s.basicColor.handle = loadShader(graphics, "shaders/basicColor.vert", "shaders/basicColor.frag", coloredVertexAttribs);
        s.basicColor.mvpMatrix = graphics.getUniformLocation(s.basicColor.handle.get(), "mvpMatrix");
        s.basicColor.alpha = graphics.getUniformLocation(s.basicColor.handle.get(), "alpha");

        s.basicTexture.handle = loadShader(graphics, "shaders/basicTexture.vert", "shaders/basicTexture.frag", texturedVertexAttribs);
        s.basicTexture.mvpMatrix = graphics.getUniformLocation(s.basicTexture.handle.get(), "mvpMatrix");
        s.basicTexture.tint = graphics.getUniformLocation(s.basicTexture.handle.get(), "tint");

        s.unitTexture.handle = loadShader(graphics, "shaders/unitTexture.vert", "shaders/unitTexture.frag", texturedVertexAttribs);
        s.unitTexture.mvpMatrix = graphics.getUniformLocation(s.unitTexture.handle.get(), "mvpMatrix");
        s.unitTexture.modelMatrix = graphics.getUniformLocation(s.unitTexture.handle.get(), "modelMatrix");
        s.unitTexture.seaLevel = graphics.getUniformLocation(s.unitTexture.handle.get(), "seaLevel");
        s.unitTexture.shade = graphics.getUniformLocation(s.unitTexture.handle.get(), "shade");

        s.unitShadow.handle = loadShader(graphics, "shaders/unitShadow.vert", "shaders/unitShadow.frag", texturedVertexAttribs);
        s.unitShadow.vpMatrix = graphics.getUniformLocation(s.unitShadow.handle.get(), "vpMatrix");
        s.unitShadow.modelMatrix = graphics.getUniformLocation(s.unitShadow.handle.get(), "modelMatrix");
        s.unitShadow.groundHeight = graphics.getUniformLocation(s.unitShadow.handle.get(), "groundHeight");

        s.unitBuild.handle = loadShader(graphics, "shaders/unitBuild.vert", "shaders/unitBuild.frag", texturedVertexAttribs);
        s.unitBuild.mvpMatrix = graphics.getUniformLocation(s.unitBuild.handle.get(), "mvpMatrix");
        s.unitBuild.unitY = graphics.getUniformLocation(s.unitBuild.handle.get(), "unitY");
        s.unitBuild.modelMatrix = graphics.getUniformLocation(s.unitBuild.handle.get(), "modelMatrix");
        s.unitBuild.seaLevel = graphics.getUniformLocation(s.unitBuild.handle.get(), "seaLevel");
        s.unitBuild.shade = graphics.getUniformLocation(s.unitBuild.handle.get(), "shade");
        s.unitBuild.percentComplete = graphics.getUniformLocation(s.unitBuild.handle.get(), "percentComplete");
        s.unitBuild.time = graphics.getUniformLocation(s.unitBuild.handle.get(), "time");

        return s;
    }

    std::string ShaderService::slurpFile(const std::string& filename)
    {
        std::ifstream inFile(filename, std::ios::binary);
        if (inFile.fail())
        {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        std::stringstream strStream;
        strStream << inFile.rdbuf();
        return strStream.str();
    }

    ShaderProgramHandle ShaderService::loadShader(
        GraphicsContext& graphics,
        const std::string& vertexShaderName,
        const std::string& fragmentShaderName,
        const std::vector<AttribMapping>& attribs)
    {
        auto vertexShaderSource = slurpFile(vertexShaderName);
        auto vertexShader = graphics.compileVertexShader(vertexShaderSource);

        auto fragmentShaderSource = slurpFile(fragmentShaderName);
        auto fragmentShader = graphics.compileFragmentShader(fragmentShaderSource);

        return graphics.linkShaderProgram(vertexShader.get(), fragmentShader.get(), attribs);
    }
}
