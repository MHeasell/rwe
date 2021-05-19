#include "UiLabel.h"

namespace rwe
{

    void UiLabel::render(UiRenderService& context) const
    {
        switch (alignment)
        {
            case Alignment::Left:
                context.drawTextWrapped(Rectangle2f::fromTopLeft(static_cast<float>(posX), posY + 12.0f, static_cast<float>(sizeX), static_cast<float>(sizeY)), text, *font);
                break;
            case Alignment::Center:
                context.drawTextCentered(static_cast<float>(posX), static_cast<float>(posY), text, *font);
                break;
        }
    }

    UiLabel::UiLabel(int posX, int posY, unsigned int sizeX, unsigned int sizeY, const std::string& text, const std::shared_ptr<SpriteSeries>& font) : UiComponent(posX, posY, sizeX, sizeY), text(text), font(font)
    {
    }

    void UiLabel::setText(const std::string& newText)
    {
        text = newText;
    }

    void UiLabel::setAlignment(Alignment newAlignment)
    {
        alignment = newAlignment;
    }
}
