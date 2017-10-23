#include <catch.hpp>
#include <rwe/tdf/SimpleTdfAdapter.h>

#include <utf8.h>

#include <ostream>
#include <vector>
#include <algorithm>

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

            TdfBlock expected({
                TdfBlockEntry("Foo", std::vector<TdfBlockEntry> {
                    TdfBlockEntry("Bar", "1"),
                    TdfBlockEntry("Baz", "2"),
                    TdfBlockEntry("Alice", "Bob")
                })
            });

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

            TdfBlock expected({
                TdfBlockEntry("Foo", std::vector<TdfBlockEntry> {
                    TdfBlockEntry("Bar", "1"),
                    TdfBlockEntry("Baz", "2"),
                    TdfBlockEntry("Alice", "Bob")
                })
            });

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

            TdfBlock expected({
                TdfBlockEntry("Foo", std::vector<TdfBlockEntry> {
                    TdfBlockEntry("Bar", "1"),
                    TdfBlockEntry("Baz", "2"),
                    TdfBlockEntry("Alice", "Bob")
                })
            });

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

            TdfBlock expected({
                TdfBlockEntry("Foo", std::vector<TdfBlockEntry> {
                    TdfBlockEntry("Bar", "1"),
                    TdfBlockEntry("Baz", "2"),
                    TdfBlockEntry("Alice", "Bob")
                })
            });

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

            TdfBlock expected({
                TdfBlockEntry("Foo", std::vector<TdfBlockEntry> {
                    TdfBlockEntry("Bar", "1"),
                    TdfBlockEntry("Baz", "2"),
                    TdfBlockEntry("Alice", "Bob")
                })
            });

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

            TdfBlock expected({
                TdfBlockEntry("Foo Bar Baz", std::vector<TdfBlockEntry> {
                    TdfBlockEntry("Item One", "The First Item"),
                    TdfBlockEntry("Item Two", "123  456"),
                    TdfBlockEntry("Item The Third", "Three ee eeee")
                })
            });

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

            TdfBlock expected({
                TdfBlockEntry("Foo", std::vector<TdfBlockEntry>{
                    TdfBlockEntry("help", "")
                })
            });

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

            TdfBlock expected({
                                  TdfBlockEntry("Foo", std::vector<TdfBlockEntry>{
                                      TdfBlockEntry("The\n        Thing", "Great")
                                  })
                              });

            auto result = parser.parse(cUtf8Begin(input), cUtf8End(input));
            REQUIRE(result == expected);
        }
    }
}
