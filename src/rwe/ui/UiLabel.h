#ifndef RWE_UILABEL_H
#define RWE_UILABEL_H

#include <rwe/ui/UiComponent.h>

namespace rwe
{
    class UiLabel : public UiComponent
    {
    private:
        std::string text;
        std::shared_ptr<SpriteSeries> font;
    public:
        UiLabel(int posX, int posY, unsigned int sizeX, unsigned int sizeY, const std::string& text,
                const std::shared_ptr<SpriteSeries>& font);

        void render(GraphicsContext& context) const override;

    };
}

#endif
