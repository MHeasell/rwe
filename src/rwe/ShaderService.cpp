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

        s.basicColor = loadShader(graphics, "shaders/basicColor.vert", "shaders/basicColor.frag", coloredVertexAttribs);
        s.basicTexture = loadShader(graphics, "shaders/basicTexture.vert", "shaders/basicTexture.frag", texturedVertexAttribs);
        s.unitColor = loadShader(graphics, "shaders/unitColor.vert", "shaders/unitColor.frag", coloredVertexAttribs);
        s.unitTexture = loadShader(graphics, "shaders/unitTexture.vert", "shaders/unitTexture.frag", texturedVertexAttribs);
        s.sprite = loadShader(graphics, "shaders/sprite.vert", "shaders/sprite.frag", texturedVertexAttribs);

        return s;
    }

    std::string ShaderService::slurpFile(const std::string& filename)
    {
        std::ifstream inFile(filename, std::ios::binary);
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
