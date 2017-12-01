#ifndef RWE_UISURFACE_H
#define RWE_UISURFACE_H

#include <boost/optional.hpp>
#include <rwe/ui/UiComponent.h>
#include <memory>

namespace rwe
{
    class UiSurface : public UiComponent
    {
    private:
        boost::optional<std::shared_ptr<Sprite>> background;

    public:
        UiSurface(int posX, int posY, unsigned int sizeX, unsigned int sizeY);
        UiSurface(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<Sprite> background);

        void render(UiRenderService& context) const override;

        void setBackground(std::shared_ptr<Sprite> newBackground);
        void clearBackground();
    };
}

#endif
