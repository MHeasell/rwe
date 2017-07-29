#ifndef RWE_STRING_H
#define RWE_STRING_H

#include <string>
#include <vector>

#include <utf8.h>

namespace rwe
{
    std::vector<std::string> utf8Split(const std::string& str, const std::vector<unsigned int>& codePoints);
    std::vector<std::string> utf8Split(const std::string& str, unsigned int codePoint);

    std::vector<std::string> split(const std::string& str, const std::vector<char>& codePoints);
    std::vector<std::string> split(const std::string& str, char codePoint);

    std::string toUpper(const std::string& str);

    utf8::iterator<std::string::const_iterator> utf8Begin(const std::string& str);
    utf8::iterator<std::string::const_iterator> utf8End(const std::string& str);

}

#endif
