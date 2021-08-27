#pragma once

#include <memory>
#include <optional>
#include <rwe/observable/BehaviorSubject.h>
#include <rwe/render/SpriteSeries.h>
#include <rwe/ui/UiComponent.h>
#include <vector>

namespace rwe
{
    class UiListBox : public UiComponent
    {
    private:
        class ListBoxUiMessageVisitor
        {
        private:
            UiListBox* listBox;

        public:
            explicit ListBoxUiMessageVisitor(UiListBox* listBox) : listBox(listBox)
            {
            }

            void operator()(const ScrollPositionMessage& msg) const
            {
                auto scrollPos = static_cast<int>(msg.scrollPosition * listBox->maxScrollPosition());
                listBox->scrollPositionSubject.next(scrollPos);
            }

            void operator()(const ScrollUpMessage& /*msg*/) const
            {
                listBox->scrollUp();
            }

            void operator()(const ScrollDownMessage& /*msg*/) const
            {
                listBox->scrollDown();
            }

            template <typename T>
            void operator()(const T& /*msg*/) const
            {
                // do nothing
            }
        };


    private:
        std::vector<std::string> items;
        Subject<bool> itemsChangedSubject;
        std::shared_ptr<SpriteSeries> font;
        BehaviorSubject<std::optional<int>> selectedIndexSubject;
        BehaviorSubject<int> scrollPositionSubject{0};

    public:
        UiListBox(int posX, int posY, int sizeX, int sizeY, std::shared_ptr<SpriteSeries> font);
        void appendItem(std::string item);

        void setSelectedItem(const std::string& item);

        void clearSelectedItem();

        void render(UiRenderService& context) const override;

        void mouseDown(MouseButtonEvent event) override;

        void mouseWheel(MouseWheelEvent event) override;

        void uiMessage(const GroupMessage& message) override;

        Observable<std::optional<int>>& selectedIndex();

        const Observable<std::optional<int>>& selectedIndex() const;

        Observable<bool>& itemsChanged();

        const Observable<bool>& itemsChanged() const;

        Observable<int>& scrollPosition();

        const Observable<int>& scrollPosition() const;

        const std::vector<std::string>& getItems();

        float getScrollPercent() const;

        float getViewportPercent() const;

    private:
        int numberOfLines() const;

        std::optional<int> pixelToLine(int y) const;

        void setScrollPosition(int newPosition);

        void setScrollPositionCentered(int newPosition);

        int maxScrollPosition() const;

        void scrollUp();

        void scrollDown();
    };
}
