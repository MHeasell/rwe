#ifndef RWE_PLAYERID_H
#define RWE_PLAYERID_H

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct PlayerIdTag;
    using PlayerId = OpaqueId<unsigned int, PlayerIdTag>;
}

#endif
