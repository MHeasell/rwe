#include <catch2/catch.hpp>
#include <rwe/ip_util.h>

using namespace std::string_literals;

namespace std
{
    template <typename A, typename B>
    std::ostream& operator<<(std::ostream& os, const std::pair<A, B>& p)
    {
        os << "{" << p.first << ", " << p.second << "}";
        return os;
    }
}

namespace rwe
{

    TEST_CASE("getHostAndPort")
    {
        SECTION("works")
        {
            REQUIRE(getHostAndPort("192.168.0.1:1234"s).value() == std::make_pair("192.168.0.1"s, "1234"s));
            REQUIRE(getHostAndPort("www.example.com:1234"s).value() == std::make_pair("www.example.com"s, "1234"s));
            REQUIRE(getHostAndPort("[2001:db8::1]:8080"s).value() == std::make_pair("2001:db8::1"s, "8080"s));
            REQUIRE(getHostAndPort("::ffff:192.168.0.1:8080"s).value() == std::make_pair("::ffff:192.168.0.1"s, "8080"s));
            REQUIRE(!getHostAndPort("not-an-address-and-port"s));
        }
    }
}
