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

    TEST_CASE("utf8Trim")
    {
        SECTION("trims leading and trailing spaces")
        {
            std::string s("    foo  ");
            utf8Trim(s);
            REQUIRE(s == "foo");
        }
    }

    TEST_CASE("endsWith")
    {
        SECTION("returns true if the string ends with the substring")
        {
            std::string s("foo.txt");
            std::string e(".txt");
            REQUIRE(endsWith(s, e));
        }

        SECTION("returns false otherwise")
        {
            std::string s("foottxt");
            std::string e(".txt");
            REQUIRE(!endsWith(s, e));
        }

        SECTION("returns false when the string is shorter")
        {
            std::string s("txt");
            std::string e(".txt");
            REQUIRE(!endsWith(s, e));
        }
    }

    TEST_CASE("startsWith")
    {
        SECTION("returns true if the string starts with the substring")
        {
            std::string s("foo.txt");
            std::string p("foo");
            REQUIRE(startsWith(s, p));
        }

        SECTION("returns false otherwise")
        {
            std::string s("fobar.txt");
            std::string p("foo");
            REQUIRE(!startsWith(s, p));
        }

        SECTION("returns false when the string is shorter")
        {
            std::string s("foo");
            std::string p("fooo");
            REQUIRE(!startsWith(s, p));
        }
    }
}
