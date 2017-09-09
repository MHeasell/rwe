#include "UiListBox.h"

namespace rwe
{

    UiListBox::UiListBox(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<SpriteSeries> font)
        : UiComponent(posX, posY, sizeX, sizeY), font(std::move(font))
    {
    }

    void UiListBox::render(GraphicsContext& context) const
    {
        auto lines = std::min<unsigned int>(numberOfLines(), items.size());

        for (unsigned int i = 0; i < lines; ++i)
        {
            float y = 12.0f + (i * 12.0f);
            auto itemIndex = scrollPosition + i;

            const auto& selectedIndexValue = selectedIndexSubject.getValue();
            if (selectedIndexValue && itemIndex == *selectedIndexValue)
            {
                context.fillColor(posX, posY + y - 11.0f, sizeX, 12.0f, Color(255, 255, 255, 31));
            }

            const auto& e = items[itemIndex];

            context.drawText(posX, posY + y, e, *font);
        }
    }

    void UiListBox::appendItem(std::string item)
    {
        items.push_back(std::move(item));
    }

    void UiListBox::mouseDown(MouseButtonEvent event)
    {
        auto line = pixelToLine(event.y);
        if (!line)
        {
            return;
        }

        auto index = *line + scrollPosition;
        if (index < items.size())
        {
            selectedIndexSubject.next(index);
        }
    }

    void UiListBox::setSelectedItem(const std::string& item)
    {
        auto it = std::find(items.begin(), items.end(), item);
        if (it != items.end())
        {
            auto index = static_cast<unsigned int>(it - items.begin());
            selectedIndexSubject.next(index);
            setScrollPositionCentered(index);
        }
    }

    void UiListBox::clearSelectedItem()
    {
        selectedIndexSubject.next(boost::none);
    }

    Observable<boost::optional<unsigned int>>& UiListBox::selectedIndex()
    {
        return selectedIndexSubject;
    }

    const Observable<boost::optional<unsigned int>>& UiListBox::selectedIndex() const
    {
        return selectedIndexSubject;
    }

    const std::vector<std::string>& UiListBox::getItems()
    {
        return items;
    }

    void UiListBox::mouseWheel(MouseWheelEvent event)
    {
        auto maxScroll = maxScrollPosition();
        if (event.y < 0)
        {
            if (scrollPosition < maxScroll - 3)
            {
                scrollPosition += 3;
            }
            else
            {
                scrollPosition = maxScroll;
            }
        }
        else if (event.y > 0)
        {
            if (scrollPosition > 3)
            {
                scrollPosition -= 3;
            }
            else
            {
                scrollPosition = 0;
            }
        }
    }

    unsigned int UiListBox::numberOfLines() const
    {
        auto lines = static_cast<unsigned int>((sizeY) / 12.0f);

        // listboxes defined in TA guis tend to be too long,
        // so shorten our number of lines by 1 to compensate.
        return lines - 1;
    }

    boost::optional<unsigned int> UiListBox::pixelToLine(int y) const
    {
        auto floatIndex = (y - posY) / 12.0f;
        if (floatIndex < 0.0f)
        {
            return boost::none;
        }

        auto index = static_cast<unsigned int>(floatIndex);
        if (index >= numberOfLines())
        {
            return boost::none;
        }

        return index;
    }

    void UiListBox::setScrollPositionCentered(unsigned int newPosition)
    {
        auto halfLines = numberOfLines() / 2;
        if (newPosition > halfLines)
        {
            setScrollPosition(newPosition - halfLines);
        }
        else
        {
            setScrollPosition(0);
        }
    }

    void UiListBox::setScrollPosition(unsigned int newPosition)
    {
        auto maxScroll = maxScrollPosition();
        if (newPosition > maxScroll)
        {
            scrollPosition = maxScroll;
        }
        else
        {
            scrollPosition = newPosition;
        }
    }

    unsigned int UiListBox::maxScrollPosition() const
    {
        auto listSize = items.size();
        auto displayLines = numberOfLines();
        if (listSize > displayLines)
        {
            return static_cast<unsigned int>(listSize - displayLines);
        }
        else
        {
            return 0;
        }
    }
}
