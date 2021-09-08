#pragma once

#include <rwe/Viewport.h>
namespace rwe
{
    class CroppedViewport : public AbstractViewport
    {
    private:
        const Viewport* base;
        int leftInset;
        int topInset;
        int rightInset;
        int bottomInset;

    public:
        CroppedViewport(const Viewport* base, int leftInset, int topInset, int rightInset, int bottomInset);
        int x() const override;
        int y() const override;
        unsigned int width() const override;
        unsigned int height() const override;
    };
}
