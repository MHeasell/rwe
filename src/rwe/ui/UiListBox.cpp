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
        unsigned int i = 0;
        for (const auto& e : items)
        {
            y += 12.0f;

            // stop one row short of the bottom
            // because listboxes defined in gui files are normally too long
            if (y + 12.0f > sizeY)
            {
                break;
            }

            if (selectedIndex && i == *selectedIndex)
            {
                context.fillColor(posX, posY + y - 11.0f, sizeX, 12.0f, Color(255, 255, 255, 31));
            }

            context.drawText(posX, posY + y, e, *font);

            ++i;
        }
    }

    void UiListBox::appendItem(std::string item)
    {
        items.push_back(std::move(item));
    }

    void UiListBox::mouseDown(MouseButtonEvent event)
    {
        auto index = static_cast<unsigned int>((event.y - posY) / 12.0f);
        if (index < items.size())
        {
            selectedIndex = index;
        }
    }
}
