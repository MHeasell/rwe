#include "CroppedViewport.h"

namespace rwe
{
    CroppedViewport::CroppedViewport(const Viewport* base, int leftInset, int topInset, int rightInset, int bottomInset) : base(base), leftInset(leftInset), topInset(topInset), rightInset(rightInset), bottomInset(bottomInset) {}

    int CroppedViewport::x() const
    {
        return base->x() + leftInset;
    }
    int CroppedViewport::y() const
    {
        return base->y() + topInset;
    }
    unsigned int CroppedViewport::width() const
    {
        return base->width() - leftInset - rightInset;
    }
    unsigned int CroppedViewport::height() const
    {
        return base->height() - topInset - bottomInset;
    }
}
