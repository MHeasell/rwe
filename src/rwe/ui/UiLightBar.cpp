#include "UiLightBar.h"

namespace rwe
{

    UiLightBar::UiLightBar(int posX, int posY, unsigned int sizeX, unsigned int sizeY, const Sprite& lightMask)
        : UiComponent(posX, posY, sizeX, sizeY), lightMask(lightMask)
    {
    }

    void UiLightBar::render(UiRenderService& context) const
    {
        auto color = percentComplete == 1.0f ? Color(83, 223, 79) : Color(255, 71, 0);
        context.fillColor(posX, posY, sizeX * percentComplete, sizeY, color);

        context.drawTextureRegion(
            posX,
            posY,
            sizeX,
            sizeY,
            lightMask.texture);
    }

    void UiLightBar::setPercentComplete(float percent)
    {
        percentComplete = percent;
    }
}
