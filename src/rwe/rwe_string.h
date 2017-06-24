#ifndef RWE_STRING_H
#define RWE_STRING_H

#include <string>
#include <vector>

namespace rwe
{
    std::vector<std::string> utf8Split(const std::string& str, const std::vector<unsigned int>& codePoints);
    std::vector<std::string> utf8Split(const std::string& str, unsigned int codePoint);
}

#endif
