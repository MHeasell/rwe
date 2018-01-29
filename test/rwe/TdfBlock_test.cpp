#include <boost/optional/optional_io.hpp>
#include <catch.hpp>
#include <rwe/tdf/TdfBlock.h>

namespace rwe
{
    std::ostream& operator<<(std::ostream& os, const TdfBlock& b)
    {
        os << "<block with " << b.entries.size() << " entries>";
        return os;
    }

    TEST_CASE("TdfBlock")
    {
        TdfBlock inner;
        inner.entries.push_back(TdfBlockEntry("asdf", "qwer"));

        TdfBlock dupe;
        dupe.entries.push_back(TdfBlockEntry("x", "2"));

        TdfBlock b;
        b.entries.push_back(TdfBlockEntry("foo", "bar"));
        b.entries.push_back(TdfBlockEntry("alice", "bob"));
        b.entries.push_back(TdfBlockEntry("whiskey", inner));
        b.entries.push_back(TdfBlockEntry("dupe", "1"));

        b.entries.push_back(TdfBlockEntry("DUPE", dupe));

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
            SECTION("finds block when a property of the same name exists")
            {
                auto block = b.findBlock("DUPE");
                REQUIRE(block);
                REQUIRE(*block == dupe);
            }
        }
    }
}
