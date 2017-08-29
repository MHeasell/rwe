#include "UiLabel.h"

namespace rwe
{

    void UiLabel::render(GraphicsContext& context) const
    {
        context.drawTextCentered(posX, posY, text, *font);
    }

    UiLabel::UiLabel(int posX, int posY, unsigned int sizeX, unsigned int sizeY, const std::string& text,
                     const std::shared_ptr<SpriteSeries>& font) : UiComponent(posX, posY, sizeX, sizeY), text(text),
                                                                  font(font)
    {}
}
