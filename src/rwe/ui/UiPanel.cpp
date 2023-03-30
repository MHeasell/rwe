#include "UiPanel.h"
#include <rwe/util/rwe_string.h>

namespace rwe
{
    UiPanel::UiPanel(int posX, int posY, unsigned int sizeX, unsigned int sizeY)
        : UiComponent(posX, posY, sizeX, sizeY)
    {
    }

    UiPanel::UiPanel(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<Sprite> background)
        : UiComponent(posX, posY, sizeX, sizeY),
          background(std::move(background))
    {
    }

    UiPanel::UiPanel(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::optional<std::shared_ptr<Sprite>> background)
        : UiComponent(posX, posY, sizeX, sizeY),
          background(background)
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
        if (background)
        {
            graphics.drawSpriteAbs(posX, posY, **background);
        }

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
        for (auto& e : children)
        {
            e->keyDown(event);
        }
    }

    void UiPanel::keyUp(KeyEvent event)
    {
        for (auto& e : children)
        {
            e->keyUp(event);
        }
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
        auto& component = children.emplace_back(std::move(c));
        component->messages().subscribe([this, &cc = *component](const auto& message) {
            GroupMessage groupMessage(this->name, cc.getGroup(), cc.getName(), message);
            this->uiMessage(groupMessage);
            this->groupMessagesSubject.next(groupMessage);
        });
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

        focusedChild = std::nullopt;
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
        children.erase(
            std::remove_if(
                children.begin(),
                children.end(),
                [&prefix](const auto& e) { return startsWith(e->getName(), prefix); }),
            children.end());
    }

    void UiPanel::setFocusByName(const std::string& name)
    {
        auto it = std::find_if(children.begin(), children.end(), [&name](const auto& c) { return c->getName() == name; });
        if (it != children.end())
        {
            setFocus(it - children.begin());
        }
    }

    Observable<GroupMessage>& UiPanel::groupMessages()
    {
        return groupMessagesSubject;
    }

    const Observable<GroupMessage>& UiPanel::groupMessages() const
    {
        return groupMessagesSubject;
    }
}
