#ifndef RWE_UILISTBOX_H
#define RWE_UILISTBOX_H

#include <boost/optional.hpp>
#include <memory>
#include <rwe/SpriteSeries.h>
#include <rwe/ui/UiComponent.h>
#include <vector>

namespace rwe
{
    class UiListBox : public UiComponent
    {
    private:
        std::vector<std::string> items;
        std::shared_ptr<SpriteSeries> font;
        boost::optional<unsigned int> selectedIndex;
    public:
        UiListBox(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<SpriteSeries> font);
        void appendItem(std::string item);

        void render(GraphicsContext& context) const override;

        void mouseDown(MouseButtonEvent event) override;
    };
}

#endif
