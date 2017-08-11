#ifndef RWE_GRAPHICSCONTEXT_H
#define RWE_GRAPHICSCONTEXT_H

#include <memory>
#include <rwe/math/Vector3f.h>
#include <SDL.h>
#include <rwe/TextureHandle.h>
#include <GL/glew.h>

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
    };
}

#endif
