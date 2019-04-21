#include "PlayerColorIndex.h"

namespace rwe
{
    PlayerColorIndex::PlayerColorIndex(unsigned int _value) : OpaqueId(_value)
    {
        if (_value >= 10u)
        {
            throw std::logic_error("Player color index out of range");
        }
    }
}
