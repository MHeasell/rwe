#include "UiLabel.h"

namespace rwe
{

    void UiLabel::render(GraphicsContext& context) const
    {
        context.drawTextWrapped(Rectangle2f::fromTopLeft(posX, posY + 12.0f, sizeX, sizeY), text, *font);
    }

    UiLabel::UiLabel(int posX, int posY, unsigned int sizeX, unsigned int sizeY, const std::string& text,
        const std::shared_ptr<SpriteSeries>& font) : UiComponent(posX, posY, sizeX, sizeY), text(text), font(font)
    {
    }

    void UiLabel::setText(const std::string& newText)
    {
        text = newText;
    }
}
