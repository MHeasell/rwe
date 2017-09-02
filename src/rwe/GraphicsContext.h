#ifndef RWE_GRAPHICSCONTEXT_H
#define RWE_GRAPHICSCONTEXT_H

#include <GL/glew.h>
#include <SDL.h>
#include <memory>
#include <rwe/SharedTextureHandle.h>
#include <rwe/Sprite.h>
#include <rwe/TextureHandle.h>
#include <rwe/math/Vector3f.h>
#include <rwe/camera/AbstractCamera.h>
#include <rwe/ColorPalette.h>
#include <rwe/SpriteSeries.h>

namespace rwe
{
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

        SharedTextureHandle createTexture(unsigned int width, unsigned int height, const std::vector<Color>& image);

        SharedTextureHandle createColorTexture(Color c);

        void drawTextureRegion(float x, float y, float width, float height, GLuint texture, float u, float v, float uw, float vh);

        void drawTextureRegion(float x, float y, float width, float height, const SharedTextureHandle& texture, float u, float v, float uw, float vh);

        void drawTexture(float x, float y, float width, float height, GLuint texture);

        void drawTexture(float x, float y, float width, float height, const SharedTextureHandle& texture);

        void drawSprite(float x, float y, const Sprite& sprite);

        void drawText(float x, float y, const std::string& text, const SpriteSeries& font);

        void drawTextCentered(float x, float y, const std::string& text, const SpriteSeries& font);

        float getTextWidth(const std::string& text, const SpriteSeries& font);

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
    };
}

#endif
