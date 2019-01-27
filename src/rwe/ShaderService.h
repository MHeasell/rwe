#ifndef RWE_SHADERSERVICE_H
#define RWE_SHADERSERVICE_H

#include <rwe/GraphicsContext.h>
#include <unordered_map>

namespace rwe
{
    struct BasicColorShader
    {
        ShaderProgramHandle handle;
        UniformLocation mvpMatrix;
        UniformLocation alpha;
    };

    struct BasicTextureShader
    {
        ShaderProgramHandle handle;
        UniformLocation mvpMatrix;
        UniformLocation alpha;
    };

    struct UnitTextureShader
    {
        ShaderProgramHandle handle;
        UniformLocation mvpMatrix;
        UniformLocation modelMatrix;
        UniformLocation seaLevel;
        UniformLocation shade;
    };

    struct UnitColorShader
    {
        ShaderProgramHandle handle;
        UniformLocation mvpMatrix;
        UniformLocation modelMatrix;
        UniformLocation seaLevel;
        UniformLocation shade;
    };

    struct UnitBuildShader
    {
        ShaderProgramHandle handle;
        UniformLocation mvpMatrix;
        UniformLocation modelMatrix;
        UniformLocation unitY;
        UniformLocation seaLevel;
        UniformLocation shade;
        UniformLocation percentComplete;
        UniformLocation time;
    };

    class ShaderService
    {
    public:
        static ShaderService createShaderService(GraphicsContext& graphics);

    private:
        static std::string slurpFile(const std::string& filename);

        static ShaderProgramHandle loadShader(GraphicsContext& graphics, const std::string& vertexShaderName, const std::string& fragmentShaderName, const std::vector<AttribMapping>& attribs);

    public:
        BasicColorShader basicColor;
        BasicTextureShader basicTexture;
        UnitColorShader unitColor;
        UnitTextureShader unitTexture;
        UnitBuildShader unitBuild;
    };
}

#endif
