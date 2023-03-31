#include "GameHash.h"

namespace rwe
{
    GameHash operator+(GameHash a, GameHash b)
    {
        return GameHash(a.value + b.value);
    }

    GameHash& operator+=(GameHash& a, const GameHash& b)
    {
        a = a + b;
        return a;
    }
}
