#pragma once

#include <rwe/ui/UiComponent.h>

namespace rwe
{
    class UiLabel : public UiComponent
    {
    public:
        enum class Alignment
        {
            Left,
            Center
        };

    private:
        std::string text;
        std::shared_ptr<SpriteSeries> font;
        Alignment alignment{Alignment::Left};

    public:
        UiLabel(int posX, int posY, unsigned int sizeX, unsigned int sizeY, const std::string& text, const std::shared_ptr<SpriteSeries>& font);

        void render(UiRenderService& context) const override;

        void setText(const std::string& newText);

        void setAlignment(Alignment newAlignment);
    };
}
