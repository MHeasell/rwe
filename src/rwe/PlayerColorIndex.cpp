#include "PlayerColorIndex.h"
#include <stdexcept>

namespace rwe
{
    PlayerColorIndex::PlayerColorIndex(int _value) : OpaqueId(_value)
    {
        if (_value >= 10 || _value < 0)
        {
            throw std::logic_error("Player color index out of range");
        }
    }
}
