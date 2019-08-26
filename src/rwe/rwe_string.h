#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <utf8.h>

namespace rwe
{
    using ConstUtf8Iterator = utf8::iterator<std::string::const_iterator>;
    using Utf8Iterator = utf8::iterator<std::string::iterator>;

    using ReverseConstUtf8Iterator = std::reverse_iterator<ConstUtf8Iterator>;
    using ReverseUtf8Iterator = std::reverse_iterator<Utf8Iterator>;

    std::vector<std::string> utf8Split(const std::string& str, const std::vector<unsigned int>& codePoints);
    std::vector<std::string> utf8Split(const std::string& str, unsigned int codePoint);
    std::optional<std::pair<std::string, std::string>> utf8SplitLast(const std::string& str, unsigned int codePoint);

    std::vector<std::string> split(const std::string& str, const std::vector<char>& codePoints);
    std::vector<std::string> split(const std::string& str, char codePoint);

    std::string toUpper(const std::string& str);

    Utf8Iterator utf8Begin(std::string& str);
    ConstUtf8Iterator utf8Begin(const std::string& str);
    ConstUtf8Iterator cUtf8Begin(const std::string& str);

    Utf8Iterator utf8End(std::string& str);
    ConstUtf8Iterator utf8End(const std::string& str);
    ConstUtf8Iterator cUtf8End(const std::string& str);

    void utf8TrimLeft(std::string& str);
    void utf8TrimRight(std::string& str);
    void utf8Trim(std::string& str);

    bool startsWith(const std::string& str, const std::string& prefix);
    bool endsWith(const std::string& str, const std::string& end);
    bool endsWithUtf8(const std::string& str, const std::string& end);

    std::string latin1ToUtf8(const std::string& str);
}
