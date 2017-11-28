#ifndef RWE_GRAPHICSCONTEXT_H
#define RWE_GRAPHICSCONTEXT_H

#include "GlMesh.h"
#include "SelectionMesh.h"
#include "ShaderHandle.h"
#include "ShaderMesh.h"
#include "ShaderProgramHandle.h"
#include <GL/glew.h>
#include <SDL.h>
#include <memory>
#include <rwe/ColorPalette.h>
#include <rwe/MapFeature.h>
#include <rwe/MapTerrain.h>
#include <rwe/Mesh.h>
#include <rwe/Sprite.h>
#include <rwe/SpriteSeries.h>
#include <rwe/TextureHandle.h>
#include <rwe/camera/AbstractCamera.h>
#include <rwe/geometry/CollisionMesh.h>
#include <rwe/math/Vector3f.h>
#include <rwe/UniformLocation.h>

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

        TextureHandle createTexture(unsigned int width, unsigned int height, const std::vector<Color>& image);

        TextureHandle createTexture(unsigned int width, unsigned int height, const Color* image);

        TextureHandle createColorTexture(Color c);

        void drawShaderMesh(
            const ShaderMesh& mesh,
            ShaderProgramIdentifier textureShader,
            ShaderProgramIdentifier colorShader,
            const Matrix4f& modelMatrix,
            const Matrix4f& mvpMatrix,
            float seaLevel);

        void drawWireframeSelectionMesh(const GlMesh& mesh, const Matrix4f& mvpMatrix, ShaderProgramIdentifier shader);

        void enableDepth();

        void enableDepthWrites();

        void disableDepthWrites();

        void enableCulling();

        void disableDepth();

        ShaderHandle compileVertexShader(const std::string& source);

        ShaderHandle compileFragmentShader(const std::string& source);

        ShaderProgramHandle linkShaderProgram(ShaderIdentifier vertexShader, ShaderIdentifier fragmentShader, const std::vector<AttribMapping>& attribs);

        void beginUnitShadow();

        void endUnitShadow();

        GlMesh createTexturedMesh(const std::vector<GlTexturedVertex>& vertices, GLenum usage);

        GlMesh createColoredMesh(const std::vector<GlColoredVertex>& vertices, GLenum usage);

        void drawLinesMesh(const GlMesh& mesh, const Matrix4f& mvpMatrix, ShaderProgramIdentifier shader);

        void drawTrisMesh(const GlMesh& mesh, const Matrix4f& mvpMatrix, ShaderProgramIdentifier shader);

        void drawSprite(const GlMesh& mesh, const Matrix4f& mvpMatrix, float alpha, ShaderProgramIdentifier shader);

        void bindShader(ShaderProgramIdentifier shader);

        void unbindShader();

        void bindTexture(TextureIdentifier texture);

        void unbindTexture();

        void enableBlending();

        void disableBlending();

        UniformLocation getUniformLocation(ShaderProgramIdentifier shader, const std::string& name);

        void setUniformFloat(UniformLocation location, float value);
        void setUniformMatrix(UniformLocation location, const Matrix4f& matrix);

        void drawTriangles(const GlMesh& mesh);
        void drawLines(const GlMesh& mesh);

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

#endif
