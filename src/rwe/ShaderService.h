#ifndef RWE_SHADERSERVICE_H
#define RWE_SHADERSERVICE_H

#include "GraphicsContext.h"
#include <unordered_map>

namespace rwe
{
    class ShaderService
    {
    public:
        static ShaderService createShaderService(GraphicsContext& graphics);

    private:
        static std::string slurpFile(const std::string& filename);

        static ShaderProgramHandle loadShader(GraphicsContext& graphics, const std::string& vertexShaderName, const std::string& fragmentShaderName, const std::vector<AttribMapping>& attribs);

    public:
        ShaderProgramHandle basicColor;
        ShaderProgramHandle basicTexture;
        ShaderProgramHandle unitColor;
        ShaderProgramHandle unitTexture;
        ShaderProgramHandle sprite;
    };
}

#endif
