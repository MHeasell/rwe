#pragma once

#include <rwe/render/GraphicsContext.h>
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
        UniformLocation tint;
    };

    struct MapTerrainShader
    {
        ShaderProgramHandle handle;
        UniformLocation mvpMatrix;
    };

    struct UnitTextureShader
    {
        ShaderProgramHandle handle;
        UniformLocation mvpMatrix;
        UniformLocation modelMatrix;
        UniformLocation seaLevel;
        UniformLocation shade;
    };

    struct UnitShadowShader
    {
        ShaderProgramHandle handle;
        UniformLocation vpMatrix;
        UniformLocation modelMatrix;
        UniformLocation groundHeight;
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
        MapTerrainShader mapTerrain;
        UnitTextureShader unitTexture;
        UnitShadowShader unitShadow;
        UnitBuildShader unitBuild;
    };
}
