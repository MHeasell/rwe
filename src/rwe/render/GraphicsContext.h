#pragma once

#include "TextureArrayHandle.h"
#include <GL/glew.h>
#include <SDL.h>
#include <memory>
#include <rwe/ColorPalette.h>
#include <rwe/Mesh.h>
#include <rwe/camera/AbstractCamera.h>
#include <rwe/geometry/CollisionMesh.h>
#include <rwe/math/Vector3f.h>
#include <rwe/render/GlMesh.h>
#include <rwe/render/ShaderHandle.h>
#include <rwe/render/ShaderProgramHandle.h>
#include <rwe/render/Sprite.h>
#include <rwe/render/SpriteSeries.h>
#include <rwe/render/TextureHandle.h>
#include <rwe/render/UniformLocation.h>
#include <rwe/sim/MapFeature.h>
#include <rwe/sim/MapTerrain.h>

namespace rwe
{
#pragma pack(1)
    struct GlTexturedVertex
    {
        GLfloat x;
        GLfloat y;
        GLfloat z;
        GLfloat u;
        GLfloat v;

        GlTexturedVertex() = default;
        GlTexturedVertex(const Vector3f& pos, const Vector2f& texCoord);
    };

    struct GlTextureArrayVertex
    {
        GLfloat x;
        GLfloat y;
        GLfloat z;
        GLfloat u;
        GLfloat v;
        GLfloat w;

        GlTextureArrayVertex() = default;
        GlTextureArrayVertex(const Vector3f& pos, const Vector3f& texCoord);
    };

    struct GlTexturedNormalVertex
    {
        GLfloat x;
        GLfloat y;
        GLfloat z;
        GLfloat u;
        GLfloat v;
        GLfloat nx;
        GLfloat ny;
        GLfloat nz;

        GlTexturedNormalVertex() = default;
        GlTexturedNormalVertex(const Vector3f& pos, const Vector2f& texCoord, const Vector3f& normal);
    };

    struct GlColoredVertex
    {
        GLfloat x;
        GLfloat y;
        GLfloat z;
        GLfloat r;
        GLfloat g;
        GLfloat b;

        GlColoredVertex() = default;
        GlColoredVertex(const Vector3f& pos, const Vector3f& color);
    };

    struct GlColoredNormalVertex
    {
        GLfloat x;
        GLfloat y;
        GLfloat z;
        GLfloat r;
        GLfloat g;
        GLfloat b;
        GLfloat nx;
        GLfloat ny;
        GLfloat nz;

        GlColoredNormalVertex() = default;
        GlColoredNormalVertex(const Vector3f& pos, const Vector3f& color, const Vector3f& normal);
    };
#pragma pack()

    struct AttribMapping
    {
        std::string name;
        GLuint location;

        AttribMapping(const std::string& name, GLuint location);
    };

    class GraphicsException : public std::runtime_error
    {
    public:
        explicit GraphicsException(const std::string& __arg);

        explicit GraphicsException(const char* string);
    };

    class OpenGlException : public GraphicsException
    {
    public:
        explicit OpenGlException(GLenum error);
    };

    class GraphicsContext
    {
    public:
        void clear();

        TextureHandle createTexture(const Grid<Color>& image);

        TextureHandle createTexture(int width, int height, const std::vector<Color>& image);

        TextureHandle createTexture(int width, int height, const Color* image);

        TextureHandle createColorTexture(Color c);

        TextureArrayHandle createTextureArray(int width, int height, int mipMapLevels, std::vector<Color>& images);

        void enableDepthBuffer();

        void disableDepthBuffer();

        void enableDepthWrites();

        void disableDepthWrites();

        void enableDepthTest();

        void disableDepthTest();

        void enableCulling();


        ShaderHandle compileVertexShader(const std::string& source);

        ShaderHandle compileFragmentShader(const std::string& source);

        ShaderProgramHandle linkShaderProgram(ShaderIdentifier vertexShader, ShaderIdentifier fragmentShader, const std::vector<AttribMapping>& attribs);

        void enableColorBuffer();
        void disableColorBuffer();
        void enableStencilBuffer();
        void useStencilBufferForWrites();
        void useStencilBufferAsMask();
        void clearStencilBuffer();
        void disableStencilBuffer();

        GlMesh createTexturedMesh(const std::vector<GlTexturedVertex>& vertices, GLenum usage);

        GlMesh createTextureArrayMesh(const std::vector<GlTextureArrayVertex>& vertices, GLenum usage);

        GlMesh createColoredMesh(const std::vector<GlColoredVertex>& vertices, GLenum usage);

        GlMesh createTexturedNormalMesh(const std::vector<GlTexturedNormalVertex>& vertices, GLenum usage);

        GlMesh createColoredNormalMesh(const std::vector<GlColoredNormalVertex>& vertices, GLenum usage);

        void bindShader(ShaderProgramIdentifier shader);

        void unbindShader();

        void bindTexture(TextureIdentifier texture);

        void bindTextureArray(TextureArrayIdentifier texture);

        void unbindTexture();

        void unbindTextureArray();

        void enableBlending();

        void disableBlending();

        UniformLocation getUniformLocation(ShaderProgramIdentifier shader, const std::string& name);

        void setUniformFloat(UniformLocation location, float value);
        void setUniformVec4(UniformLocation location, float a, float b, float c, float d);
        void setUniformMatrix(UniformLocation location, const Matrix4f& matrix);
        void setUniformBool(UniformLocation location, bool value);

        void drawTriangles(const GlMesh& mesh);
        void drawLines(const GlMesh& mesh);
        void drawLineLoop(const GlMesh& mesh);

        Sprite createSprite(const Rectangle2f& bounds, const Rectangle2f& textureRegion, const SharedTextureHandle& texture);

        GlMesh createUnitTexturedQuad(const Rectangle2f& textureRegion);

        void setViewport(int x, int y, int width, int height);

    private:
        ShaderHandle compileShader(GLenum shaderType, const std::string& source);

        VboHandle genBuffer();
        VaoHandle genVertexArray();
        void bindBuffer(GLenum type, VboIdentifier id);
        void unbindBuffer(GLenum type);
        void bindVertexArray(VaoIdentifier id);
        void unbindVertexArray();
        void drawMesh(
            GLenum mode,
            const GlMesh& mesh,
            const Matrix4f& mvpMatrix,
            ShaderProgramIdentifier shader);
        void drawUnitMesh(
            const GlMesh& mesh,
            const Matrix4f& modelMatrix,
            const Matrix4f& mvpMatrix,
            float seaLevel,
            ShaderProgramIdentifier shader);
    };
}
