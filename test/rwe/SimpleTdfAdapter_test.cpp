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

            SimpleTdfAdapter adapter;
            TdfParser<ConstUtf8Iterator> parser;
            parser.parse(cUtf8Begin(input), cUtf8End(input), adapter);

            CAPTURE(adapter.getRoot());
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

            SimpleTdfAdapter adapter;
            TdfParser<ConstUtf8Iterator> parser;
            parser.parse(cUtf8Begin(input), cUtf8End(input), adapter);

            CAPTURE(adapter.getRoot());
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

            SimpleTdfAdapter adapter;
            TdfParser<ConstUtf8Iterator> parser;
            parser.parse(cUtf8Begin(input), cUtf8End(input), adapter);

            CAPTURE(adapter.getRoot());
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

            SimpleTdfAdapter adapter;
            TdfParser<ConstUtf8Iterator> parser;
            parser.parse(cUtf8Begin(input), cUtf8End(input), adapter);

            CAPTURE(adapter.getRoot());
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

            SimpleTdfAdapter adapter;
            TdfParser<ConstUtf8Iterator> parser;
            parser.parse(cUtf8Begin(input), cUtf8End(input), adapter);

            CAPTURE(adapter.getRoot());
            REQUIRE(adapter.getRoot() == expected);
        }
    }
}
