#ifndef RWE_UILIGHTBAR_H
#define RWE_UILIGHTBAR_H

#include <memory>
#include <rwe/ui/UiComponent.h>

namespace rwe
{
    class UiLightBar : public UiComponent
    {
    private:
        unsigned int numberOfSections{38};

        std::shared_ptr<Sprite> lightMask;

        float percentComplete{0.0f};

    public:
        UiLightBar(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<Sprite> lightMask);

        void render(UiRenderService& context) const override;

        void setPercentComplete(float percent);
    };
}

#endif
