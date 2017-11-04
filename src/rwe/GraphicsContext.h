#ifndef RWE_GRAPHICSCONTEXT_H
#define RWE_GRAPHICSCONTEXT_H

#include <GL/glew.h>
#include <SDL.h>
#include <memory>
#include <rwe/ColorPalette.h>
#include <rwe/MapFeature.h>
#include <rwe/MapTerrain.h>
#include <rwe/Mesh.h>
#include <rwe/TextureHandle.h>
#include <rwe/Sprite.h>
#include <rwe/SpriteSeries.h>
#include <rwe/camera/AbstractCamera.h>
#include <rwe/math/Vector3f.h>
#include <rwe/geometry/CollisionMesh.h>
#include "ShaderHandle.h"
#include "ShaderProgramHandle.h"
#include "ShaderMesh.h"
#include "SelectionMesh.h"

namespace rwe
{
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
        void drawTriangle(const Vector3f& a, const Vector3f& b, const Vector3f& c);

        void setBackgroundColor(float r, float g, float b);

        void clear();

        TextureHandle createTexture(const Grid<Color>& image);

        TextureHandle createTexture(unsigned int width, unsigned int height, const std::vector<Color>& image);

        TextureHandle createTexture(unsigned int width, unsigned int height, const Color* image);

        TextureHandle createColorTexture(Color c);

        void drawTextureRegion(float x, float y, float width, float height, const TextureRegion& texture);

        void drawTextureRegion(float x, float y, float width, float height, TextureIdentifier texture, float u, float v, float uw, float vh);

        void drawTextureRegion(float x, float y, float width, float height, const SharedTextureHandle& texture, float u, float v, float uw, float vh);

        void drawTexture(float x, float y, float width, float height, TextureIdentifier texture);

        void drawTexture(float x, float y, float width, float height, const SharedTextureHandle& texture);

        void drawSprite(float x, float y, const Sprite& sprite);

        void drawText(float x, float y, const std::string& text, const SpriteSeries& font);

        void drawTextWrapped(Rectangle2f area, const std::string& text, const SpriteSeries& font);

        void drawTextCentered(float x, float y, const std::string& text, const SpriteSeries& font);

        float getTextWidth(const std::string& text, const SpriteSeries& font);

        template <typename It>
        float getTextWidth(It begin, It end, const SpriteSeries& font);

        template <typename It>
        It findEndOfWord(It it, It end);

        void applyCamera(const AbstractCamera& camera);

        /**
         * Multiplies the current OpenGL matrix with the specified matrix.
         * If the current OpenGL matrix is C and the coordinates to be transformed are v,
         * meaning that the current transformation would be C * v,
         * then calling this with an argument M replaces the current transformation with (C * M) * v.
         * Intuitively this means that any transformation you pass in here
         * will be done directly on the coordinates,
         * and the existing tranformation will be done on the result of that.
         */
        void multiplyMatrix(const Matrix4f& m);

        void pushMatrix();

        void popMatrix();

        void fillColor(float x, float y, float width, float height, Color color);

        void drawMapTerrain(const MapTerrain& terrain, unsigned int x, unsigned int y, unsigned int width, unsigned int height);

        void drawFeature(const MapFeature& feature);

        void drawStandingSprite(const Vector3f& position, const Sprite& sprite);

        void drawStandingSprite(const Vector3f& position, const Sprite& sprite, float alpha);

        void drawMesh(const Mesh& mesh);

        void drawShaderMesh(
            const ShaderMesh& mesh,
            ShaderProgramIdentifier textureShader,
            ShaderProgramIdentifier colorShader,
            const Matrix4f& modelMatrix,
            const Matrix4f& viewMatrix,
            const Matrix4f& projectionMatrix);

        void drawWireframeSelectionMesh(
            const VisualSelectionMesh& mesh,
            const Matrix4f& modelMatrix,
            const Matrix4f& viewMatrix,
            const Matrix4f& projectionMatrix,
            ShaderProgramIdentifier shader);

        void enableDepth();

        void enableCulling();

        void disableDepth();

        ShaderHandle compileVertexShader(const std::string& source);

        ShaderHandle compileFragmentShader(const std::string& source);

        ShaderProgramHandle linkShaderProgram(ShaderIdentifier vertexShader, ShaderIdentifier fragmentShader, const std::vector<AttribMapping>& attribs);

        ShaderMesh convertMesh(const Mesh& mesh);

        VisualSelectionMesh createSelectionMesh(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& d);

    private:
        ShaderHandle compileShader(GLenum shaderType, const std::string& source);
    };

    template <typename It>
    float GraphicsContext::getTextWidth(It it, It end, const SpriteSeries& font)
    {
        float width = 0;
        for (; it != end; ++it)
        {
            auto ch = *it;

            if (ch > font.sprites.size())
            {
                ch = 0;
            }

            const auto& sprite = font.sprites[ch];

            width += sprite.bounds.right();
        }

        return width;
    }

    template <typename It>
    It GraphicsContext::findEndOfWord(It it, It end)
    {
        return std::find_if(it, end, [](int ch) { return ch == ' '; });
    }
}

#endif
