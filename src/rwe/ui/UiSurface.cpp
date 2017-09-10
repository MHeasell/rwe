#include "UiSurface.h"

namespace rwe
{
    UiSurface::UiSurface(int posX, int posY, unsigned int sizeX, unsigned int sizeY)
        : UiComponent(posX, posY, sizeX, sizeY)
    {
    }

    UiSurface::UiSurface(int posX, int posY, unsigned int sizeX, unsigned int sizeY, const Sprite& background)
        : UiComponent(posX, posY, sizeX, sizeY), background(background)
    {
    }

    void UiSurface::render(GraphicsContext& context) const
    {
        if (background)
        {
            float scale = std::min(
                static_cast<float>(sizeX) / background->bounds.width(),
                static_cast<float>(sizeY) / background->bounds.height());

            float x = static_cast<float>(posX) + (static_cast<float>(sizeX) / 2.0f) - ((background->bounds.width() * scale) / 2.0f);
            float y = static_cast<float>(posY) + (static_cast<float>(sizeY) / 2.0f) - ((background->bounds.height() * scale) / 2.0f);

            context.pushMatrix();
            context.multiplyMatrix(Matrix4f::translation(Vector3f(x, y, 0.0f)));
            context.multiplyMatrix(Matrix4f::scale(Vector3f(scale, scale, 1.0f)));
            context.drawSprite(0.0f, 0.0f, *background);
            context.popMatrix();
        }
    }

    void UiSurface::setBackground(const Sprite& newBackground)
    {
        background = newBackground;
    }

    void UiSurface::clearBackground()
    {
        background = boost::none;
    }
}
