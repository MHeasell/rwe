#include "UiListBox.h"

namespace rwe
{

    UiListBox::UiListBox(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<SpriteSeries> font)
            : UiComponent(posX, posY, sizeX, sizeY), font(std::move(font))
    {
    }

    void UiListBox::render(GraphicsContext& context) const
    {
        float y = 0.0f;
        for (const auto& e : items)
        {
            y += 12.0f;

            // stop one row short of the bottom
            // because listboxes defined in gui files are normally too long
            if (y + 12.0f > sizeY)
            {
                break;
            }

            context.drawText(posX, posY + y, e, *font);
        }
    }

    void UiListBox::appendItem(std::string item)
    {
        items.push_back(std::move(item));
    }
}
