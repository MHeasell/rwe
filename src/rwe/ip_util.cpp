#include "ip_util.h"
#include <rwe/rwe_string.h>

namespace rwe
{
    std::optional<std::pair<std::string, std::string>> getHostAndPort(const std::string& input)
    {
        auto hostAndPort = utf8SplitLast(input, U':');
        if (!hostAndPort)
        {
            return std::nullopt;
        }

        if (startsWith(hostAndPort->first, "[") && endsWithUtf8(hostAndPort->first, "]"))
        {
            auto newHost = std::string(++hostAndPort->first.begin(), --hostAndPort->first.end());
            return std::make_pair(newHost, hostAndPort->second);
        }

        return hostAndPort;
    }
}
