#include <catch.hpp>
#include <rwe/SimpleTdfAdapter.h>

#include <utf8.h>

#include <ostream>
#include <vector>
#include <algorithm>

namespace rwe
{
    TEST_CASE("SimpleTdfAdapter")
    {
        SimpleTdfAdapter adapter;
        TdfParser<ConstUtf8Iterator> parser;

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

            std::vector<SimpleTdfAdapter::BlockEntry> expected{
                SimpleTdfAdapter::BlockEntry("Foo", std::vector<SimpleTdfAdapter::BlockEntry> {
                    SimpleTdfAdapter::BlockEntry("Bar", "1"),
                    SimpleTdfAdapter::BlockEntry("Baz", "2"),
                    SimpleTdfAdapter::BlockEntry("Alice", "Bob")
                })
            };

            parser.parse(cUtf8Begin(input), cUtf8End(input), adapter);

            REQUIRE(adapter.getRoot() == expected);
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

            std::vector<SimpleTdfAdapter::BlockEntry> expected{
                SimpleTdfAdapter::BlockEntry("Foo", std::vector<SimpleTdfAdapter::BlockEntry> {
                    SimpleTdfAdapter::BlockEntry("Bar", "1"),
                    SimpleTdfAdapter::BlockEntry("Baz", "2"),
                    SimpleTdfAdapter::BlockEntry("Alice", "Bob")
                })
            };

            parser.parse(cUtf8Begin(input), cUtf8End(input), adapter);

            REQUIRE(adapter.getRoot() == expected);
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

            std::vector<SimpleTdfAdapter::BlockEntry> expected{
                SimpleTdfAdapter::BlockEntry("Foo", std::vector<SimpleTdfAdapter::BlockEntry> {
                    SimpleTdfAdapter::BlockEntry("Bar", "1"),
                    SimpleTdfAdapter::BlockEntry("Baz", "2"),
                    SimpleTdfAdapter::BlockEntry("Alice", "Bob")
                })
            };

            parser.parse(cUtf8Begin(input), cUtf8End(input), adapter);

            REQUIRE(adapter.getRoot() == expected);
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

            std::vector<SimpleTdfAdapter::BlockEntry> expected{
                SimpleTdfAdapter::BlockEntry("Foo", std::vector<SimpleTdfAdapter::BlockEntry> {
                    SimpleTdfAdapter::BlockEntry("Bar", "1"),
                    SimpleTdfAdapter::BlockEntry("Baz", "2"),
                    SimpleTdfAdapter::BlockEntry("Alice", "Bob")
                })
            };

            parser.parse(cUtf8Begin(input), cUtf8End(input), adapter);

            REQUIRE(adapter.getRoot() == expected);
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

            std::vector<SimpleTdfAdapter::BlockEntry> expected{
                SimpleTdfAdapter::BlockEntry("Foo", std::vector<SimpleTdfAdapter::BlockEntry> {
                    SimpleTdfAdapter::BlockEntry("Bar", "1"),
                    SimpleTdfAdapter::BlockEntry("Baz", "2"),
                    SimpleTdfAdapter::BlockEntry("Alice", "Bob")
                })
            };

            parser.parse(cUtf8Begin(input), cUtf8End(input), adapter);

            REQUIRE(adapter.getRoot() == expected);
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

            std::vector<SimpleTdfAdapter::BlockEntry> expected{
                SimpleTdfAdapter::BlockEntry("Foo Bar Baz", std::vector<SimpleTdfAdapter::BlockEntry> {
                    SimpleTdfAdapter::BlockEntry("Item One", "The First Item"),
                    SimpleTdfAdapter::BlockEntry("Item Two", "123  456"),
                    SimpleTdfAdapter::BlockEntry("Item The Third", "Three ee eeee")
                })
            };

            parser.parse(cUtf8Begin(input), cUtf8End(input), adapter);

            REQUIRE(adapter.getRoot() == expected);
        }
    }
}
