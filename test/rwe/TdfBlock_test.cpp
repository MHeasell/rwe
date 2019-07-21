#include <rwe/optional_io.h>
#include <catch2/catch.hpp>
#include <rwe/tdf/TdfBlock.h>

namespace rwe
{
    std::ostream& operator<<(std::ostream& os, const TdfBlock& b);

    TEST_CASE("TdfBlock")
    {
        TdfBlock inner;
        inner.insertOrAssignProperty("asdf", "qwer");

        TdfBlock dupe;
        dupe.insertOrAssignProperty("x", "2");

        TdfBlock b;
        b.insertOrAssignProperty("foo", "bar");
        b.insertOrAssignProperty("alice", "bob");
        b.insertOrAssignBlock("whiskey", inner);
        b.insertOrAssignProperty("dupe", "1");

        b.insertOrAssignBlock("DUPE", dupe);

        SECTION("findValue")
        {
            SECTION("returns the value matching the key")
            {
                auto value = b.findValue("alice");
                REQUIRE(value);
                REQUIRE(value->get() == "bob");
            }
            SECTION("ignores case")
            {
                auto value = b.findValue("AlIcE");
                REQUIRE(value);
                REQUIRE(value->get() == "bob");
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
                REQUIRE(block->get() == inner);
            }
            SECTION("returns the block matching the key")
            {
                auto block = b.findBlock("wHIskEY");
                REQUIRE(block->get() == inner);
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
            SECTION("finds block when a property of the same name exists")
            {
                auto block = b.findBlock("DUPE");
                REQUIRE(block);
                REQUIRE(block->get() == dupe);
            }
        }
    }
}
