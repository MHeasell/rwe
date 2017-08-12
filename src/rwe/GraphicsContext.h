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

        void drawTexture(int x, int y, unsigned int width, unsigned int height, GLuint texture);

        void drawTexture(int x, int y, unsigned int width, unsigned int height, const SharedTextureHandle& texture);

        void drawSprite(int x, int y, const Sprite& sprite);
    };
}

#endif
