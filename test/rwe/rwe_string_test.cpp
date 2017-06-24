#include <catch.hpp>

#include <rwe/rwe_string.h>

namespace rwe
{
    TEST_CASE("utf8Split")
    {
        SECTION("works on empty string")
        {
            std::string s;
            std::vector<std::string> expected { "" };

            auto actual = utf8Split(s, '/');

            REQUIRE(actual == expected);
        }

        SECTION("splits a utf8 string on a delimiter")
        {
            std::string s("foo/bar/baz");
            std::vector<std::string> expected { "foo", "bar", "baz" };

            auto actual = utf8Split(s, '/');

            REQUIRE(actual == expected);
        }

        SECTION("works when the delimiter is not found")
        {
            std::string s("foo.bar.baz");
            std::vector<std::string> expected { "foo.bar.baz" };

            auto actual = utf8Split(s, '/');

            REQUIRE(actual == expected);
        }
    }
}
