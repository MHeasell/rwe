#include "UiPanel.h"
#include <rwe/rwe_string.h>

namespace rwe
{

    UiPanel::UiPanel(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<Sprite> background)
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

    void UiPanel::render(UiRenderService& graphics) const
    {
        graphics.drawSpriteAbs(posX, posY, sizeX, sizeY, *background);

        graphics.pushMatrix();
        graphics.multiplyMatrix(Matrix4f::translation(Vector3f(posX, posY, 0.0f)));

        for (const auto& i : children)
        {
            i->render(graphics);
        }

        graphics.popMatrix();
    }

    void UiPanel::mouseDown(MouseButtonEvent event)
    {
        event.x -= posX;
        event.y -= posY;

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

        event.x -= posX;
        event.y -= posY;

        (*focusedChild)->mouseUp(event);
    }

    void UiPanel::keyDown(KeyEvent event)
    {
        if (!focusedChild)
        {
            return;
        }

        (*focusedChild)->keyDown(event);
    }

    void UiPanel::keyUp(KeyEvent event)
    {
        if (!focusedChild)
        {
            return;
        }

        (*focusedChild)->keyUp(event);
    }

    void UiPanel::mouseMove(MouseMoveEvent event)
    {
        UiComponent::mouseMove(event);

        event.x -= posX;
        event.y -= posY;

        for (auto& e : children)
        {
            e->mouseMove(event);
        }
    }

    void UiPanel::focus()
    {
        if (!focusedChild)
        {
            (*focusedChild)->focus();
        }
    }

    void UiPanel::unfocus()
    {
        if (!focusedChild)
        {
            (*focusedChild)->unfocus();
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
            (*focusedChild)->unfocus();
        }

        focusedChild = children[controlIndex].get();
        (*focusedChild)->focus();
    }

    void UiPanel::clearFocus()
    {
        if (focusedChild)
        {
            (*focusedChild)->unfocus();
        }

        focusedChild = boost::none;
    }

    void UiPanel::update(float dt)
    {
        for (auto& c : children)
        {
            c->update(dt);
        }
    }

    void UiPanel::mouseWheel(MouseWheelEvent event)
    {
        if (!focusedChild)
        {
            return;
        }

        (*focusedChild)->mouseWheel(event);
    }

    void UiPanel::uiMessage(const GroupMessage& message)
    {
        for (auto& c : children)
        {
            c->uiMessage(message);
        }
    }

    std::vector<std::unique_ptr<UiComponent>>& UiPanel::getChildren()
    {
        return children;
    }

    void UiPanel::removeChildrenWithPrefix(const std::string& prefix)
    {
        auto it = children.rbegin();
        auto end = children.rend();

        for (; it != end; ++it)
        {
            if (startsWith((*it)->getName(), prefix))
            {
                if (focusedChild && focusedChild == it->get())
                {
                    focusedChild = boost::none;
                }

                children.erase(--(it.base()));
            }
        }
    }
}
