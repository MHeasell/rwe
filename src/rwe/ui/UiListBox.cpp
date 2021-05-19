#include "UiListBox.h"

namespace rwe
{
    UiListBox::UiListBox(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<SpriteSeries> font)
        : UiComponent(posX, posY, sizeX, sizeY), font(std::move(font))
    {
        scrollPosition().subscribe([this](const auto& /*scrollPos*/) {
            ScrollPositionMessage m{getViewportPercent(), getScrollPercent()};
            messagesSubject.next(m);
        });
        itemsChanged().subscribe([this](const auto& /*nothing*/) {
            ScrollPositionMessage m{getViewportPercent(), getScrollPercent()};
            messagesSubject.next(m);
        });
    }

    void UiListBox::render(UiRenderService& context) const
    {
        auto lines = std::min<unsigned int>(numberOfLines(), static_cast<unsigned int>(items.size()));

        for (unsigned int i = 0; i < lines; ++i)
        {
            int y = 12 + (i * 12);
            auto itemIndex = scrollPositionSubject.getValue() + i;

            const auto& selectedIndexValue = selectedIndexSubject.getValue();
            if (selectedIndexValue && itemIndex == *selectedIndexValue)
            {
                context.fillColor(static_cast<float>(posX), posY + y - 11.0f, static_cast<float>(sizeX), 12.0f, Color(255, 255, 255, 31));
            }

            const auto& e = items[itemIndex];

            context.drawText(posX, posY + y, e, *font);
        }
    }

    void UiListBox::appendItem(std::string item)
    {
        items.push_back(std::move(item));
        itemsChangedSubject.next(true);
    }

    void UiListBox::mouseDown(MouseButtonEvent event)
    {
        auto line = pixelToLine(event.y);
        if (!line)
        {
            return;
        }

        auto index = *line + scrollPositionSubject.getValue();
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
        selectedIndexSubject.next(std::nullopt);
    }

    Observable<std::optional<unsigned int>>& UiListBox::selectedIndex()
    {
        return selectedIndexSubject;
    }

    const Observable<std::optional<unsigned int>>& UiListBox::selectedIndex() const
    {
        return selectedIndexSubject;
    }

    Observable<bool>& UiListBox::itemsChanged()
    {
        return itemsChangedSubject;
    }

    const Observable<bool>& UiListBox::itemsChanged() const
    {
        return itemsChangedSubject;
    }

    const std::vector<std::string>& UiListBox::getItems()
    {
        return items;
    }

    void UiListBox::mouseWheel(MouseWheelEvent event)
    {
        if (event.y < 0)
        {
            scrollDown();
        }
        else if (event.y > 0)
        {
            scrollUp();
        }
    }

    unsigned int UiListBox::numberOfLines() const
    {
        auto lines = static_cast<unsigned int>((sizeY) / 12.0f);

        // listboxes defined in TA guis tend to be too long,
        // so shorten our number of lines by 1 to compensate.
        return lines - 1;
    }

    std::optional<unsigned int> UiListBox::pixelToLine(int y) const
    {
        auto floatIndex = (y - posY) / 12.0f;
        if (floatIndex < 0.0f)
        {
            return std::nullopt;
        }

        auto index = static_cast<unsigned int>(floatIndex);
        if (index >= numberOfLines())
        {
            return std::nullopt;
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
            scrollPositionSubject.next(maxScroll);
        }
        else
        {
            scrollPositionSubject.next(newPosition);
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

    Observable<unsigned int>& UiListBox::scrollPosition()
    {
        return scrollPositionSubject;
    }

    const Observable<unsigned int>& UiListBox::scrollPosition() const
    {
        return scrollPositionSubject;
    }

    float UiListBox::getScrollPercent() const
    {
        auto scrollIndex = scrollPositionSubject.getValue();
        auto maxScrollIndex = maxScrollPosition();
        if (maxScrollIndex == 0)
        {
            return 0.0f;
        }

        return static_cast<float>(scrollIndex) / static_cast<float>(maxScrollIndex);
    }

    float UiListBox::getViewportPercent() const
    {
        auto itemCount = items.size();
        auto linesCount = numberOfLines();

        if (linesCount >= itemCount)
        {
            return 1.0f;
        }

        return static_cast<float>(linesCount) / static_cast<float>(itemCount);
    }

    void UiListBox::uiMessage(const GroupMessage& message)
    {
        if (message.controlName == name)
        {
            return;
        }

        if (message.group != group)
        {
            return;
        }

        std::visit(ListBoxUiMessageVisitor(this), message.message);
    }

    void UiListBox::scrollUp()
    {
        if (scrollPositionSubject.getValue() > 3)
        {
            scrollPositionSubject.next(scrollPositionSubject.getValue() - 3);
        }
        else
        {
            scrollPositionSubject.next(0);
        }
    }

    void UiListBox::scrollDown()
    {
        auto maxScroll = maxScrollPosition();
        if (scrollPositionSubject.getValue() < maxScroll - 3)
        {
            scrollPositionSubject.next(scrollPositionSubject.getValue() + 3);
        }
        else
        {
            scrollPositionSubject.next(maxScroll);
        }
    }
}
