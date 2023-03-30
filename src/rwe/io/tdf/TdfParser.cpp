#include "TdfParser.h"
#include <rwe/util/rwe_string.h>

#include <utf8.h>

namespace rwe
{
    TdfParserException::TdfParserException(std::size_t line, std::size_t column, const char* message) : runtime_error(message), line(line), column(column) {}

    TdfParserException::TdfParserException(std::size_t line, std::size_t column, const std::string& message) : runtime_error(message), line(line), column(column) {}
}
