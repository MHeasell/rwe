#include "io_utils.h"

namespace rwe
{
    std::string readNullTerminatedString(std::istream& stream)
    {
        char c;

        std::string result;

        for (;;)
        {
            stream.read(&c, 1);
            if (stream.fail() || c == '\0')
            {
                return result;
            }

            result.push_back(c);
        }
    }
}
