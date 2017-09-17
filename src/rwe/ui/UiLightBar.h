#ifndef RWE_UILIGHTBAR_H
#define RWE_UILIGHTBAR_H

#include <rwe/ui/UiComponent.h>

namespace rwe
{
    class UiLightBar : public UiComponent
    {
    private:
        unsigned int numberOfSections{38};

        Sprite lightMask;

        float percentComplete{0.0f};

    public:
        UiLightBar(int posX, int posY, unsigned int sizeX, unsigned int sizeY, const Sprite& lightMask);

        void render(GraphicsContext& context) const override;

        void setPercentComplete(float percent);
    };
}

#endif
