#ifndef RWE_OCTILEDISTANCE_H
#define RWE_OCTILEDISTANCE_H

namespace rwe
{
    /** A distance in grid squares separated into straight and diagonal components. */
    struct OctileDistance
    {
        unsigned int straight;
        unsigned int diagonal;

        OctileDistance(unsigned int straight, unsigned int diagonal);

        bool operator==(const OctileDistance& rhs) const;

        bool operator!=(const OctileDistance& rhs) const;
    };
}

#endif
