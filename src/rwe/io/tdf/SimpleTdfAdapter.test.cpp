#include <catch2/catch.hpp>
#include <rwe/io/tdf/SimpleTdfAdapter.h>

#include <utf8.h>

#include <algorithm>
#include <ostream>
#include <vector>

namespace rwe
{
    TEST_CASE("SimpleTdfAdapter")
    {
        TdfParser<ConstUtf8Iterator, TdfBlock> parser(new SimpleTdfAdapter);

        SECTION("works for simple TDFs")
        {
            std::string input = R"TDF(
[Foo]
{
    Bar=1;
    Baz=2;
    Alice=Bob;
}
)TDF";

            auto expected = makeTdfBlock({{"Foo", makeTdfBlock({{"Bar", "1"}, {"Baz", "2"}, {"Alice", "Bob"}})}});

            auto result = parser.parse(cUtf8Begin(input), cUtf8End(input));

            REQUIRE(result == expected);
        }

        SECTION("supports spaces around equals signs")
        {
            std::string input = R"TDF(
[Foo]
{
    Bar = 1;
    Baz = 2;
    Alice = Bob;
}
)TDF";

            auto expected = makeTdfBlock({{"Foo", makeTdfBlock({{"Bar", "1"}, {"Baz", "2"}, {"Alice", "Bob"}})}});

            auto result = parser.parse(cUtf8Begin(input), cUtf8End(input));

            REQUIRE(result == expected);
        }

        SECTION("supports line comments")
        {
            std::string input = R"TDF(
[Foo]
{
    Bar = 1; // one

    // next item is two
    Baz = 2;
    Alice = Bob;
}
)TDF";

            auto expected = makeTdfBlock({{"Foo", makeTdfBlock({{"Bar", "1"}, {"Baz", "2"}, {"Alice", "Bob"}})}});

            auto result = parser.parse(cUtf8Begin(input), cUtf8End(input));

            REQUIRE(result == expected);
        }

        SECTION("supports block comments")
        {
            std::string input = R"TDF(
    [Foo] /* foooo! */
    {
        Bar = 1; /* this is important */

        /* ignore these:
        Alpha=Beta;
        Charlie=Delta;
        */

        Baz = 2;
        Alice = Bob;
    }
    )TDF";

            auto expected = makeTdfBlock({{"Foo", makeTdfBlock({{"Bar", "1"}, {"Baz", "2"}, {"Alice", "Bob"}})}});

            auto result = parser.parse(cUtf8Begin(input), cUtf8End(input));

            REQUIRE(result == expected);
        }

        SECTION("supports block comments inside things")
        {
            std::string input = R"TDF(
    [Fo/*ooooo*/o]
    {
        Bar = 1;
        Baz = 2;
        Ali/*ii*/ce = Bo/*ooo*/b;
    }
    )TDF";

            auto expected = makeTdfBlock({{"Foo", makeTdfBlock({{"Bar", "1"}, {"Baz", "2"}, {"Alice", "Bob"}})}});

            auto result = parser.parse(cUtf8Begin(input), cUtf8End(input));

            REQUIRE(result == expected);
        }

        SECTION("supports items with spaces in them")
        {
            std::string input = R"TDF(
    [   Foo Bar Baz   ]
    {
        Item One = The First Item;
        Item Two =     123  456  ;
        Item The Third=Three ee eeee  ;
    }
    )TDF";

            auto expected = makeTdfBlock({{"Foo Bar Baz", makeTdfBlock({{"Item One", "The First Item"}, {"Item Two", "123  456"}, {"Item The Third", "Three ee eeee"}})}});

            auto result = parser.parse(cUtf8Begin(input), cUtf8End(input));

            REQUIRE(result == expected);
        }

        SECTION("supports properties with empty values")
        {

            std::string input = R"TDF(
    [Foo]
    {
        help=;
    }
    )TDF";

            auto expected = makeTdfBlock({{"Foo", makeTdfBlock({{"help", ""}})}});

            auto result = parser.parse(cUtf8Begin(input), cUtf8End(input));
            REQUIRE(result == expected);
        }

        // This test is sort of a kludge to help with parsing ARMSCORP.FBI,
        // which mistakenly has the semicolon at the beginning of the property value
        // on line 50.
        SECTION("supports property names with newlines in them")
        {

            std::string input = R"TDF(
    [Foo]
    {
        The
        Thing=Great;
    }
    )TDF";

            auto expected = makeTdfBlock({{"Foo", makeTdfBlock({{"The\n        Thing", "Great"}})}});

            auto result = parser.parse(cUtf8Begin(input), cUtf8End(input));
            REQUIRE(result == expected);
        }


        SECTION("gives precedence to the last instance of a property")
        {
            std::string input = R"TDF(
[COMMON]
        {
        id=0;
        assoc=0;
        name=Selmap.GUI;
        xpos=84;
        ypos=12;
        width=494;  // width and height specified twice!
        height=420;
        width=640;  // this one is correct
        height=480;
        }
            )TDF";

            // clang-format off
            auto expected = makeTdfBlock({{"COMMON", makeTdfBlock({
                {"id", "0"},
                {"assoc", "0"},
                {"name", "Selmap.GUI"},
                {"xpos", "84"},
                {"ypos", "12"},
                {"width", "640"},
                {"height", "480"}
            })}});
            // clang-format on

            auto result = parser.parse(cUtf8Begin(input), cUtf8End(input));
            REQUIRE(result == expected);
        }

        SECTION("copes with empty statements")
        {
            // seen in V maps pack. Empty space followed by semicolon.
            std::string input = R"TDF(
[GlobalHeader]
    {
    missionname=[V] A Better Fate;
    missiondescription=13 x 8. W:+2-19. Improved version of Slated Fate;
    planet=Slate;
    missionhint=; ;
    brief=17Brief.txt;
    narration=20x20 32mb 2 and 4 player or up to 4, 2 man teams;
    glamour=ARM01.pcx;
    }
            )TDF";

            auto expected = makeTdfBlock({{"GlobalHeader", makeTdfBlock({{"missionname", "[V] A Better Fate"}, {"missiondescription", "13 x 8. W:+2-19. Improved version of Slated Fate"}, {"planet", "Slate"}, {"missionhint", ""}, {"brief", "17Brief.txt"}, {"narration", "20x20 32m 2 and 4 player or up to 4, 2 man teams"}, {"glamour", "ARM01.pcx"}})}});

            auto result = parser.parse(cUtf8Begin(input), cUtf8End(input));
        }
    }
}
