#ifndef RWE_UIPANEL_H
#define RWE_UIPANEL_H

#include <GL/glew.h>
#include <boost/optional.hpp>
#include <rwe/GraphicsContext.h>
#include <rwe/TextureHandle.h>
#include <rwe/ui/UiComponent.h>

namespace rwe
{
    class UiPanel : public UiComponent
    {
    private:
        std::shared_ptr<Sprite> background;
        std::vector<std::unique_ptr<UiComponent>> children;
        boost::optional<UiComponent*> focusedChild{boost::none};

    public:
        UiPanel(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<Sprite> background);

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
    };
}

#endif
