#ifndef RWE_GRAPHICSCONTEXT_H
#define RWE_GRAPHICSCONTEXT_H

#include <GL/glew.h>
#include <SDL.h>
#include <memory>
#include <rwe/SharedTextureHandle.h>
#include <rwe/Sprite.h>
#include <rwe/TextureHandle.h>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    class GraphicsContext
    {
    public:
        void drawTriangle(const Vector3f& a, const Vector3f& b, const Vector3f& c);

        void setBackgroundColor(float r, float g, float b);

        void clear();

        TextureHandle createTexture(const SDL_Surface& surface);

        void drawTexture(float x, float y, float width, float height, GLuint texture);

        void drawTexture(float x, float y, float width, float height, const SharedTextureHandle& texture);

        void drawSprite(float x, float y, const Sprite& sprite);
    };
}

#endif
