#pragma once

#include <optional>
#include <rwe/observable/Subject.h>
#include <rwe/ui/UiComponent.h>

namespace rwe
{
    class UiPanel : public UiComponent
    {
    private:
        std::optional<std::shared_ptr<Sprite>> background;
        std::vector<std::unique_ptr<UiComponent>> children;
        std::optional<UiComponent*> focusedChild{std::nullopt};
        Subject<GroupMessage> groupMessagesSubject;

    public:
        UiPanel(int posX, int posY, int sizeX, int sizeY);
        UiPanel(int posX, int posY, int sizeX, int sizeY, std::shared_ptr<Sprite> background);
        UiPanel(int posX, int posY, int sizeX, int sizeY, std::optional<std::shared_ptr<Sprite>> background);

        UiPanel(const UiPanel&) = delete;
        UiPanel& operator=(const UiPanel&) = delete;

        UiPanel(UiPanel&& panel) noexcept;

        UiPanel& operator=(UiPanel&& panel) noexcept;

        ~UiPanel() override = default;

        void render(UiRenderService& graphics) const override;

        void mouseDown(MouseButtonEvent event) override;

        void mouseUp(MouseButtonEvent event) override;

        void keyDown(KeyEvent event) override;

        void keyUp(KeyEvent event) override;

        void mouseMove(MouseMoveEvent event) override;

        void mouseWheel(MouseWheelEvent event) override;

        void update(float dt) override;

        void uiMessage(const GroupMessage& message) override;

        void focus() override;

        void unfocus() override;

        void appendChild(std::unique_ptr<UiComponent>&& c);

        void removeChildrenWithPrefix(const std::string& prefix);

        std::vector<std::unique_ptr<UiComponent>>& getChildren();

        void setFocus(std::size_t controlIndex);

        void clearFocus();

        void setFocusByName(const std::string& name);

        Observable<GroupMessage>& groupMessages();

        const Observable<GroupMessage>& groupMessages() const;

        template <typename T>
        std::optional<std::reference_wrapper<const T>> find(const std::string& name) const
        {
            static_assert(std::is_base_of_v<UiComponent, T>);
            auto it = std::find_if(children.begin(), children.end(), [&name](const auto& x) {
                return dynamic_cast<const T*>(x.get()) != nullptr && x->getName() == name;
            });
            if (it == children.end())
            {
                return std::nullopt;
            }
            return dynamic_cast<const T&>(**it);
        }

        template <typename T>
        std::optional<std::reference_wrapper<T>> find(const std::string& name)
        {
            auto component = static_cast<const UiPanel*>(this)->find<T>(name);
            if (!component)
            {
                return std::nullopt;
            }

            return const_cast<T&>(component->get());
        }

        template <typename T>
        void forAll(const std::function<void(T&)>& f)
        {
            static_assert(std::is_base_of_v<UiComponent, T>);
            for (auto& c : children)
            {
                if (auto cc = dynamic_cast<T*>(c.get()); cc != nullptr)
                {
                    f(*cc);
                }
            }
        }
    };
}
