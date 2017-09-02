#include "UiPanel.h"

namespace rwe
{

    UiPanel::UiPanel(int posX, int posY, unsigned int sizeX, unsigned int sizeY, SharedTextureHandle background)
            : UiComponent(posX, posY, sizeX, sizeY),
              background(std::move(background))
    {
    }

    UiPanel::UiPanel(UiPanel&& panel) noexcept
            : UiComponent(panel.posX, panel.posY, panel.sizeX, panel.sizeY),
              background(std::move(panel.background)),
              children(std::move(panel.children)),
              focusedChild(std::move(panel.focusedChild))
    {
    }

    UiPanel& UiPanel::operator=(UiPanel&& panel) noexcept
    {
        posX = panel.posX;
        posY = panel.posY;
        sizeX = panel.sizeX;
        sizeY = panel.sizeY;
        background = panel.background;
        children = std::move(panel.children);
        focusedChild = std::move(panel.focusedChild);

        return *this;
    }

    void UiPanel::render(GraphicsContext& graphics) const
    {
        graphics.drawTexture(0, 0, sizeX, sizeY, background);
        for (const auto& i : children)
        {
            i->render(graphics);
        }
    }

    void UiPanel::mouseDown(MouseButtonEvent event)
    {
        auto size = children.size();
        for (std::size_t i = 0; i < size; ++i)
        {
            auto& c = children[i];
            if (c->contains(event.x, event.y))
            {
                setFocus(i);
                c->mouseDown(event);
                return;
            }
        }
    }

    void UiPanel::mouseUp(MouseButtonEvent event)
    {
        if (!focusedChild)
        {
            return;
        }

        children[*focusedChild]->mouseUp(event);
    }

    void UiPanel::keyDown(KeyEvent event)
    {
        if (!focusedChild)
        {
            return;
        }

        children[*focusedChild]->keyDown(event);
    }

    void UiPanel::keyUp(KeyEvent event)
    {
        if (!focusedChild)
        {
            return;
        }

        children[*focusedChild]->keyUp(event);
    }

    void UiPanel::mouseMove(MouseMoveEvent event)
    {
        UiComponent::mouseMove(event);
        for (auto& e : children)
        {
            e->mouseMove(event);
        }
    }

    void UiPanel::focus()
    {
        if (!focusedChild)
        {
            children[*focusedChild]->focus();
        }
    }

    void UiPanel::unfocus()
    {
        if (!focusedChild)
        {
            children[*focusedChild]->unfocus();
        }
    }

    void UiPanel::appendChild(std::unique_ptr<UiComponent>&& c)
    {
        children.push_back(std::move(c));
    }

    void UiPanel::setFocus(std::size_t controlIndex)
    {
        assert(controlIndex < children.size());

        if (focusedChild)
        {
            children[*focusedChild]->unfocus();
        }

        focusedChild = controlIndex;
        children[*focusedChild]->focus();
    }

    void UiPanel::clearFocus()
    {
        if (focusedChild)
        {
            children[*focusedChild]->unfocus();
        }

        focusedChild = boost::none;
    }
}
