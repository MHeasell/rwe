#ifndef RWE_OVERLOADED_H
#define RWE_OVERLOADED_H

template <typename... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <typename... Ts>
overloaded(Ts...)->overloaded<Ts...>;

#endif
