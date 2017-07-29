#include "rwe_string.h"

#include <algorithm>
#include <utf8.h>
#include <cctype>

namespace rwe
{
    template <typename It, typename C>
    std::vector<std::string> splitInternal(It begin, It end, const std::vector<C>& codePoints)
    {
        std::vector<std::string> v;

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
    };

    std::vector<std::string> utf8Split(const std::string& str, const std::vector<unsigned int>& codePoints)
    {
        utf8::iterator<std::string::const_iterator> begin(str.begin(), str.begin(), str.end());
        utf8::iterator<std::string::const_iterator> end(str.end(), str.begin(), str.end());

        return splitInternal(begin, end, codePoints);
    }

    std::vector<std::string> utf8Split(const std::string& str, unsigned int codePoint)
    {
        std::vector<unsigned int> v{codePoint};
        return utf8Split(str, v);
    }

    std::vector<std::string> split(const std::string& str, const std::vector<char>& codePoints)
    {

        auto begin = str.begin();
        auto end = str.end();

        return splitInternal(begin, end, codePoints);
    }

    std::vector<std::string> split(const std::string& str, char codePoint)
    {
        std::vector<char> v{codePoint};
        return split(str, v);
    }

    std::string toUpper(const std::string& str)
    {
        std::string copy(str);
        std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c) { return std::toupper(c); });
        return copy;
    }

    utf8ConstIterator utf8Begin(const std::string& str)
    {
        return utf8::iterator<std::string::const_iterator>(str.begin(), str.begin(), str.end());
    }

    utf8ConstIterator utf8End(const std::string& str)
    {
        return utf8::iterator<std::string::const_iterator>(str.end(), str.begin(), str.end());
    }
}
