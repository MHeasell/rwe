#include "UiLabel.h"

namespace rwe
{

    void UiLabel::render(GraphicsContext& context) const
    {
        switch (alignment)
        {
            case Alignment::Left:
                context.drawTextWrapped(Rectangle2f::fromTopLeft(posX, posY + 12.0f, sizeX, sizeY), text, *font);
                break;
            case Alignment::Center:
                context.drawTextCentered(posX, posY, text, *font);
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
