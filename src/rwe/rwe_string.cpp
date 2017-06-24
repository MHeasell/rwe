#include "rwe_string.h"

#include <algorithm>
#include <utf8.h>

namespace rwe
{
    std::vector<std::string> utf8Split(const std::string& str, const std::vector<unsigned int>& codePoints)
    {
        std::vector<std::string> v;

        utf8::iterator<std::string::const_iterator> begin(str.begin(), str.begin(), str.end());
        utf8::iterator<std::string::const_iterator> end(str.end(), str.begin(), str.end());

        while (true)
        {
            auto it = std::find_first_of(begin, end, codePoints.begin(), codePoints.end());
            v.emplace_back(begin, it);
            if (it == end)
            {
                break;
            }

            begin = ++it;
        }

        return v;
    }

    std::vector<std::string> utf8Split(const std::string& str, unsigned int codePoint)
    {
        std::vector<unsigned int> v {codePoint};
        return utf8Split(str, v);
    }
}
