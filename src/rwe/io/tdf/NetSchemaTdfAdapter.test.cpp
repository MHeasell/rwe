#include <catch2/catch.hpp>
#include <rwe/io/tdf/NetSchemaTdfAdapter.h>

#include <utf8.h>

#include <algorithm>
#include <ostream>
#include <vector>

namespace rwe
{
    TEST_CASE("NetSchemaTdfAdapter")
    {
        TdfParser<std::string::const_iterator, bool> parser(new NetSchemaTdfAdapter);

        SECTION("no schema, no network property")
        {
            std::string input = R"TDF(
[Foo]
{
    Bar=1;
    Baz=2;
    Alice=Bob;
}
)TDF";

            bool expected = false;
            bool result = parser.parse(input.cbegin(), input.cend());

            REQUIRE(result == expected);
        }

        SECTION("no schema, with network property")
        {
            std::string input = R"TDF(
[Foo]
{
    Type=Network 1;
    Baz=2;
    Alice=Bob;
}
)TDF";

            bool expected = false;
            bool result = parser.parse(input.cbegin(), input.cend());

            REQUIRE(result == expected);
        }

        SECTION("with schema, no network property")
        {
            std::string input = R"TDF(
[Foo]
    {
    Bar=1;
    Baz=2;
    [Schema 1]
        {
        SurfaceMetal=4;
        
        [specials]
            {
            [special0]
                {
                specialwhat=StartPos1;
                XPos=2260;
                ZPos=4054;
                }
            }
        }
    }
)TDF";

            bool expected = false;
            bool result = parser.parse(input.cbegin(), input.cend());

            REQUIRE(result == expected);
        }

        SECTION("with schema, network property but not in schema")
        {
            std::string input = R"TDF(
[Foo]
    {
    Bar=1;
    Baz=2;
    Type=Network 1;
    [Schema 1]
        {
        SurfaceMetal=4;
        
        [specials]
            {
            [special0]
                {
                specialwhat=StartPos1;
                XPos=2260;
                ZPos=4054;
                }
            }
        }
    }
)TDF";

            bool expected = false;
            bool result = parser.parse(input.cbegin(), input.cend());

            REQUIRE(result == expected);
        }

        SECTION("schema with network property before nested block")
        {
            std::string input = R"TDF(
[Foo]
    {
    Bar=1;
    Baz=2;
    [Schema 1]
        {
        SurfaceMetal=4;
        Type=Network 1;
        [specials]
            {
            [special0]
                {
                specialwhat=StartPos1;
                XPos=2260;
                ZPos=4054;
                }
            }
        }
    }
)TDF";

            bool expected = true;
            bool result = parser.parse(input.cbegin(), input.cend());

            REQUIRE(result == expected);
        }

        SECTION("schema with network property after nested block")
        {
            std::string input = R"TDF(
[Foo]
{
    Bar=1;
    Baz=2;
    [Schema 1]
    {
        SurfaceMetal=4;
        [specials]
        {
            [special0]
            {
                specialwhat=StartPos1;
                XPos=2260;
                ZPos=4054;
            }
        }
        Type=Network 1;
    }
}
)TDF";

            bool expected = true;
            bool result = parser.parse(input.cbegin(), input.cend());

            REQUIRE(result == expected);
        }
    }
}
