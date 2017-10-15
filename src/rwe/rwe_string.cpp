#include "rwe_string.h"

#include <algorithm>
#include <utf8.h>
#include <cctype>

namespace rwe
{
    template <typename It, typename CIt>
    std::vector<std::string> splitInternal(It begin, It end, CIt codePointsBegin, CIt codePointsEnd)
    {
        std::vector<std::string> v;

        while (true)
        {
            auto it = std::find_first_of(begin, end, codePointsBegin, codePointsEnd);
            v.emplace_back(begin, it);
            if (it == end)
            {
                break;
            }

            begin = ++it;
        }

        return v;
    };

    template <typename It, typename Container>
    std::vector<std::string> splitInternal(It begin, It end, const Container& codePoints)
    {
        return splitInternal(begin, end, codePoints.begin(), codePoints.end());
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

    ConstUtf8Iterator utf8Begin(const std::string& str)
    {
        return utf8::iterator<std::string::const_iterator>(str.begin(), str.begin(), str.end());
    }

    ConstUtf8Iterator utf8End(const std::string& str)
    {
        return utf8::iterator<std::string::const_iterator>(str.end(), str.begin(), str.end());
    }

    ConstUtf8Iterator cUtf8Begin(const std::string& str)
    {
        return utf8Begin(str);
    }

    ConstUtf8Iterator cUtf8End(const std::string& str)
    {
        return utf8End(str);
    }

    Utf8Iterator utf8Begin(std::string& str)
    {
        return utf8::iterator<std::string::iterator>(str.begin(), str.begin(), str.end());
    }

    Utf8Iterator utf8End(std::string& str)
    {
        return utf8::iterator<std::string::iterator>(str.end(), str.begin(), str.end());
    }

    void utf8TrimLeft(std::string& str)
    {
        auto firstNonSpace = std::find_if(utf8Begin(str), utf8End(str), [](unsigned int cp) { return std::isspace(cp) == 0; });
        str.erase(str.begin(), firstNonSpace.base());
    }

    void utf8TrimRight(std::string& str)
    {
        auto it = utf8End(str);
        auto begin = utf8Begin(str);
        while (it != begin)
        {
            if (std::isspace(*--it) == 0)
            {
                ++it;
                str.erase(it.base(), str.end());
                return;
            }
        }
    }

    void utf8Trim(std::string& str)
    {
        utf8TrimRight(str);
        utf8TrimLeft(str);
    }

    bool endsWith(const std::string& str, const std::string& end)
    {
        if (str.size() < end.size())
        {
            return false;
        }

        auto it = end.rbegin();
        auto endIt = end.rend();

        auto sIt = str.rbegin();

        while (it != endIt)
        {
            if (*(it++) != *(sIt++))
            {
                return false;
            }
        }

        return true;
    }

    bool startsWith(const std::string& str, const std::string& prefix)
    {
        if (str.size() < prefix.size())
        {
            return false;
        }

        auto it = prefix.begin();
        auto endIt = prefix.end();

        auto sIt = str.begin();

        while (it != endIt)
        {
            if (*(it++) != *(sIt++))
            {
                return false;
            }
        }

        return true;
    }
}
