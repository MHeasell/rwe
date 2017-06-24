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

    TEST_CASE("split")
    {
        SECTION("works on empty string")
        {
            std::string s;
            std::vector<std::string> expected { "" };

            auto actual = split(s, '/');

            REQUIRE(actual == expected);
        }

        SECTION("splits a utf8 string on a delimiter")
        {
            std::string s("foo/bar/baz");
            std::vector<std::string> expected { "foo", "bar", "baz" };

            auto actual = split(s, '/');

            REQUIRE(actual == expected);
        }

        SECTION("works when the delimiter is not found")
        {
            std::string s("foo.bar.baz");
            std::vector<std::string> expected { "foo.bar.baz" };

            auto actual = split(s, '/');

            REQUIRE(actual == expected);
        }
    }

    TEST_CASE("toUpper")
    {
        SECTION("converts a string to uppercase")
        {
            REQUIRE(toUpper(std::string("Foo bAr baZ")) == std::string("FOO BAR BAZ"));
        }
    }
}
