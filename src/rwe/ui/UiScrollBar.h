#ifndef RWE_UISCROLLBAR_H
#define RWE_UISCROLLBAR_H

#include <memory>
#include <rwe/SpriteSeries.h>
#include <rwe/ui/UiComponent.h>

namespace rwe
{
    class UiScrollBar : public UiComponent
    {
        std::shared_ptr<SpriteSeries> sprites;

    public:
        UiScrollBar(
            int posX,
            int posY,
            unsigned int sizeX,
            unsigned int sizeY,
            std::shared_ptr<SpriteSeries> sprites);

        void render(GraphicsContext& context) const override;
    };
}

#endif
