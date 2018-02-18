#ifndef RWE_OCTILEDISTANCE_H
#define RWE_OCTILEDISTANCE_H

namespace rwe
{
    /** A distance in grid squares separated into straight and diagonal components. */
    struct OctileDistance
    {
        unsigned int straight{0};
        unsigned int diagonal{0};

        static OctileDistance fromXAndY(unsigned int x, unsigned int y);

        OctileDistance() = default;
        OctileDistance(unsigned int straight, unsigned int diagonal);

        bool operator==(const OctileDistance& rhs) const;

        bool operator!=(const OctileDistance& rhs) const;

        bool operator<(const OctileDistance& rhs) const;

        bool operator>(const OctileDistance& rhs) const;

        bool operator<=(const OctileDistance& rhs) const;

        bool operator>=(const OctileDistance& rhs) const;

        OctileDistance operator+(const OctileDistance& rhs) const;

        /**
         * Returns the distance as a float.
         * Assumes that the length of the side of a square is 1,
         * and that we are dealing with regular old Euclidian space.
         */
        float asFloat() const;
    };
}

#endif
