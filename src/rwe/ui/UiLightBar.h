#pragma once

#include <memory>
#include <rwe/ui/UiComponent.h>

namespace rwe
{
    class UiLightBar : public UiComponent
    {
    private:
        int numberOfSections{38};

        std::shared_ptr<Sprite> lightMask;

        float percentComplete{0.0f};

    public:
        UiLightBar(int posX, int posY, int sizeX, int sizeY, std::shared_ptr<Sprite> lightMask);

        void render(UiRenderService& context) const override;

        void setPercentComplete(float percent);
    };
}
