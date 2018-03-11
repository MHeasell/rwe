#ifndef RWE_OPTIONAL_IO_H
#define RWE_OPTIONAL_IO_H

#include <functional>
#include <iostream>
#include <optional>

namespace std
{
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const std::reference_wrapper<T>& r)
    {
        os << r.get();
        return os;
    }

    template <typename T>
    std::ostream& operator<<(std::ostream& os, const std::optional<T>& opt)
    {
        if (opt)
        {
            os << *opt;
        }
        else
        {
            os << "--";
        }

        return os;
    }
}

#endif
