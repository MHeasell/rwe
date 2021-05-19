#include "UiLightBar.h"

namespace rwe
{

    UiLightBar::UiLightBar(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<Sprite> lightMask)
        : UiComponent(posX, posY, sizeX, sizeY), lightMask(std::move(lightMask))
    {
    }

    void UiLightBar::render(UiRenderService& context) const
    {
        auto color = percentComplete == 1.0f ? Color(83, 223, 79) : Color(255, 71, 0);
        context.fillColor(static_cast<float>(posX), static_cast<float>(posY), sizeX * percentComplete, static_cast<float>(sizeY), color);

        context.drawSpriteAbs(posX, posY, sizeX, sizeY, *lightMask);
    }

    void UiLightBar::setPercentComplete(float percent)
    {
        percentComplete = percent;
    }
}
