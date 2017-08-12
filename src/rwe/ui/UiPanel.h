#ifndef RWE_UIPANEL_H
#define RWE_UIPANEL_H

#include <boost/optional.hpp>
#include <GL/glew.h>
#include <rwe/SharedTextureHandle.h>
#include <rwe/GraphicsContext.h>
#include <rwe/ui/UiComponent.h>

namespace rwe
{
    class UiPanel : public UiComponent
    {
    private:
        SharedTextureHandle background;
        std::vector<std::unique_ptr<UiComponent>> children;
        boost::optional<std::size_t> focusedChild{boost::none};

    public:
        UiPanel(int posX, int posY, unsigned int sizeX, unsigned int sizeY, SharedTextureHandle background)
            : UiComponent(posX, posY, sizeX, sizeY),
              background(std::move(background))
        {
        }

        UiPanel(const UiPanel&) = delete;
        UiPanel& operator=(const UiPanel&) = delete;

        UiPanel(UiPanel&& panel) noexcept
            : UiComponent(panel.posX, panel.posY, panel.sizeX, panel.sizeY),
              background(std::move(panel.background)),
              children(std::move(panel.children)),
              focusedChild(std::move(panel.focusedChild))
        {
        }

        UiPanel& operator=(UiPanel&& panel) noexcept
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

        ~UiPanel() override = default;

        void render(GraphicsContext& graphics) const override
        {
            graphics.drawTexture(0, 0, sizeX, sizeY, background);
            for (const auto& i : children)
            {
                i->render(graphics);
            }
        }

        bool mouseDown(UiMouseButtonEvent event) override
        {
            auto size = children.size();
            for (std::size_t i = 0; i < size; ++i)
            {
                auto& c = children[i];
                if (c->contains(event.x, event.y))
                {
                    setFocus(i);
                    return c->mouseDown(event);
                }
            }

            return false;
        }

        bool mouseUp(UiMouseButtonEvent event) override
        {
            if (!focusedChild)
            {
                return false;
            }

            return children[*focusedChild]->mouseUp(event);
        }

        bool keyDown(UiKeyEvent event) override
        {
            if (!focusedChild)
            {
                return false;
            }

            return children[*focusedChild]->keyDown(event);
        }

        bool keyUp(UiKeyEvent event) override
        {
            if (!focusedChild)
            {
                return false;
            }

            return children[*focusedChild]->keyUp(event);
        }

        void focus() override
        {
            if (!focusedChild)
            {
                children[*focusedChild]->focus();
            }
        }

        void unfocus() override
        {
            if (!focusedChild)
            {
                children[*focusedChild]->unfocus();
            }
        }

        template <typename T, typename... Args>
        void emplaceChild(Args&&... args)
        {
            //auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
            //children.push_back(std::move(ptr));
        }

    private:
        void setFocus(std::size_t controlIndex)
        {
            assert(controlIndex < children.size());

            if (focusedChild)
            {
                children[*focusedChild]->unfocus();
            }

            focusedChild = controlIndex;
            children[*focusedChild]->focus();
        }

        void clearFocus()
        {
            if (focusedChild)
            {
                children[*focusedChild]->unfocus();
            }

            focusedChild = boost::none;
        }
    };
}

#endif
