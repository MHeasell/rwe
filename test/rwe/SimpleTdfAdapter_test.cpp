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

            std::vector<SimpleTdfAdapter::BlockEntry> expected {
                SimpleTdfAdapter::BlockEntry("Foo", std::vector<SimpleTdfAdapter::BlockEntry> {
                    SimpleTdfAdapter::BlockEntry("Bar", "1"),
                    SimpleTdfAdapter::BlockEntry("Baz", "2"),
                    SimpleTdfAdapter::BlockEntry("Alice", "Bob")
                })
            };

            SimpleTdfAdapter adapter;
            TdfParser<utf8ConstIterator> parser;
            parser.parse(utf8Begin(input), utf8End(input), adapter);

            CAPTURE(adapter.getRoot());
            REQUIRE(adapter.getRoot() == expected);
        }
    }
}
