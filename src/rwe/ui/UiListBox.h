#ifndef RWE_UILISTBOX_H
#define RWE_UILISTBOX_H

#include <boost/optional.hpp>
#include <memory>
#include <rwe/SpriteSeries.h>
#include <rwe/observable/BehaviorSubject.h>
#include <rwe/ui/UiComponent.h>
#include <vector>

namespace rwe
{
    class UiListBox : public UiComponent
    {
    private:
        class ListBoxUiMessageVisitor : public boost::static_visitor<>
        {
        private:
            UiListBox* listBox;

        public:
            explicit ListBoxUiMessageVisitor(UiListBox* listBox) : listBox(listBox)
            {
            }

            void operator()(const ScrollPositionMessage& msg) const
            {
                auto scrollPos = static_cast<unsigned int>(msg.scrollPosition * listBox->maxScrollPosition());
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
        };


    private:
        std::vector<std::string> items;
        std::shared_ptr<SpriteSeries> font;
        BehaviorSubject<boost::optional<unsigned int>> selectedIndexSubject;
        BehaviorSubject<unsigned int> scrollPositionSubject{0};

    public:
        UiListBox(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<SpriteSeries> font);
        void appendItem(std::string item);

        void setSelectedItem(const std::string& item);

        void clearSelectedItem();

        void render(GraphicsContext& context) const override;

        void mouseDown(MouseButtonEvent event) override;

        void mouseWheel(MouseWheelEvent event) override;

        void uiMessage(const GroupMessage& message) override;

        Observable<boost::optional<unsigned int>>& selectedIndex();

        const Observable<boost::optional<unsigned int>>& selectedIndex() const;

        Observable<unsigned int>& scrollPosition();

        const Observable<unsigned int>& scrollPosition() const;

        const std::vector<std::string>& getItems();

        float getScrollPercent() const;

        float getViewportPercent() const;

    private:
        unsigned int numberOfLines() const;

        boost::optional<unsigned int> pixelToLine(int y) const;

        void setScrollPosition(unsigned int newPosition);

        void setScrollPositionCentered(unsigned int newPosition);

        unsigned int maxScrollPosition() const;

        void scrollUp();

        void scrollDown();
    };
}

#endif
