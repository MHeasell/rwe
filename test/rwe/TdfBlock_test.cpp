#include <catch.hpp>
#include <rwe/tdf/TdfBlock.h>

namespace rwe
{
    TEST_CASE("TdfBlock")
    {
        TdfBlock inner;
        inner.entries.push_back(TdfBlockEntry("asdf", "qwer"));

        TdfBlock b;
        b.entries.push_back(TdfBlockEntry("foo", "bar"));
        b.entries.push_back(TdfBlockEntry("alice", "bob"));
        b.entries.push_back(TdfBlockEntry("whiskey", inner));

        SECTION("findValue")
        {
            SECTION("returns the value matching the key")
            {
                auto value = b.findValue("alice");
                REQUIRE(value);
                REQUIRE(*value == "bob");
            }
            SECTION("ignores case")
            {
                auto value = b.findValue("AlIcE");
                REQUIRE(value);
                REQUIRE(*value == "bob");
            }
            SECTION("returns none if the key does not exist")
            {
                auto value = b.findValue("fob");
                REQUIRE(!value);
            }
            SECTION("returns none if the key is a block")
            {
                auto value = b.findValue("whiskey");
                REQUIRE(!value);
            }
        }

        SECTION("findBlock")
        {
            SECTION("returns the block matching the key")
            {
                auto block = b.findBlock("whiskey");
                REQUIRE(*block == inner);
            }
            SECTION("returns the block matching the key")
            {
                auto block = b.findBlock("wHIskEY");
                REQUIRE(*block == inner);
            }
            SECTION("returns none if the key does not exist")
            {
                auto block = b.findBlock("whoskey");
                REQUIRE(!block);
            }
            SECTION("returns none if the key is a value")
            {
                auto block = b.findBlock("foo");
                REQUIRE(!block);
            }
        }
    }
}
