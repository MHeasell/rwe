#ifndef RWE_UISURFACE_H
#define RWE_UISURFACE_H

#include <rwe/ui/UiComponent.h>
#include <boost/optional.hpp>

namespace rwe
{
    class UiSurface : public UiComponent
    {
    private:
        boost::optional<Sprite> background;

    public:
        UiSurface(int posX, int posY, unsigned int sizeX, unsigned int sizeY);
        UiSurface(int posX, int posY, unsigned int sizeX, unsigned int sizeY, const Sprite& background);

        void render(GraphicsContext& context) const override;

        void setBackground(const Sprite& newBackground);
        void clearBackground();
    };
}

#endif
