#include <catch.hpp>
#include <rwe/tdf/ListTdfAdapter.h>

#include <utf8.h>

#include <algorithm>
#include <ostream>
#include <vector>

namespace rwe
{
    TEST_CASE("ListTdfAdapter")
    {
        TdfParser<ConstUtf8Iterator, std::vector<TdfBlock>> parser(new ListTdfAdapter);

        SECTION("works for simple TDF lists")
        {
            std::string input = R"TDF(
[GADGET1]
{
    Foo=1;
}
[GADGET1]
{
    Foo=2;
}
[GADGET1]
{
    Foo=3;
    Bar=Baz;
}
)TDF";

            std::vector<TdfBlock> expected{
                makeTdfBlock({{"Foo", "1"}}),
                makeTdfBlock({{"Foo", "2"}}),
                makeTdfBlock({{"Foo", "3"}, {"Bar", "Baz"}}),
            };

            auto result = parser.parse(cUtf8Begin(input), cUtf8End(input));

            REQUIRE(result == expected);
        }
    }
}
