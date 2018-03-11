#include "UiSurface.h"

namespace rwe
{
    UiSurface::UiSurface(int posX, int posY, unsigned int sizeX, unsigned int sizeY)
        : UiComponent(posX, posY, sizeX, sizeY)
    {
    }

    UiSurface::UiSurface(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<Sprite> background)
        : UiComponent(posX, posY, sizeX, sizeY), background(std::move(background))
    {
    }

    void UiSurface::render(UiRenderService& context) const
    {
        if (background)
        {
            const auto& bg = **background;
            float scale = std::min(
                static_cast<float>(sizeX) / bg.bounds.width(),
                static_cast<float>(sizeY) / bg.bounds.height());

            float x = static_cast<float>(posX) + (static_cast<float>(sizeX) / 2.0f) - ((bg.bounds.width() * scale) / 2.0f);
            float y = static_cast<float>(posY) + (static_cast<float>(sizeY) / 2.0f) - ((bg.bounds.height() * scale) / 2.0f);

            context.pushMatrix();
            context.multiplyMatrix(Matrix4f::translation(Vector3f(x, y, 0.0f)));
            context.multiplyMatrix(Matrix4f::scale(Vector3f(scale, scale, 1.0f)));
            context.drawSprite(0.0f, 0.0f, bg);
            context.popMatrix();
        }
    }

    void UiSurface::setBackground(std::shared_ptr<Sprite> newBackground)
    {
        background = std::move(newBackground);
    }

    void UiSurface::clearBackground()
    {
        background = std::nullopt;
    }
}
