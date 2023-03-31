#include <catch2/catch.hpp>
#include <rwe/sim/GameHash_util.h>
#include <rwe/util/OpaqueId_io.h>

namespace rwe
{
    struct IdTag;
    using Id = OpaqueId<unsigned int, IdTag>;

    enum class TestEnum
    {
        CaseA,
        CaseB,
        CaseC
    };

    using TestVariant = std::variant<int, bool, float>;

    TEST_CASE("computeHashOf")
    {
        SECTION("works on GameHash")
        {
            REQUIRE(computeHashOf(GameHash(1236)) == GameHash(1236));
        }
        SECTION("works on float")
        {
            REQUIRE(computeHashOf(5.0f) == GameHash(327680));
        }
        SECTION("works on bool")
        {
            REQUIRE(computeHashOf(false) == GameHash(0));
            REQUIRE(computeHashOf(true) == GameHash(1));
        }
        SECTION("works on unsigned int")
        {
            REQUIRE(computeHashOf(1234u) == GameHash(1234));
        }
        SECTION("works on int")
        {
            REQUIRE(computeHashOf(1234) == GameHash(1234));
            REQUIRE(computeHashOf(-50) == GameHash(4294967246));
        }
        SECTION("works on string")
        {
            REQUIRE(computeHashOf(std::string("A")) == GameHash(65));
            REQUIRE(computeHashOf(std::string("fred")) == GameHash(417));
        }
        SECTION("works on char*")
        {
            REQUIRE(computeHashOf("A") == GameHash(65));
            REQUIRE(computeHashOf("fred") == GameHash(417));
        }
        SECTION("works on optional")
        {
            REQUIRE(computeHashOf(std::make_optional(38)) == GameHash(38));
            REQUIRE(computeHashOf(std::optional<int>()) == GameHash(0));
        }
        SECTION("works on vector")
        {
            std::vector<int> v{1, 2, 3, 4};
            REQUIRE(computeHashOf(v) == GameHash(10));
        }
        SECTION("works on pair")
        {
            std::pair<int, int> v{1, 2};
            REQUIRE(computeHashOf(v) == GameHash(3));
        }
        SECTION("works on VectorMap")
        {
            VectorMap<int, IdTag> v;
            v.emplace(1);
            v.emplace(2);
            v.emplace(3);
            REQUIRE(computeHashOf(v) == GameHash(774));
        }
        SECTION("works on enum")
        {
            REQUIRE(computeHashOf(TestEnum::CaseA) == GameHash(0));
            REQUIRE(computeHashOf(TestEnum::CaseB) == GameHash(1));
            REQUIRE(computeHashOf(TestEnum::CaseC) == GameHash(2));
        }
        SECTION("works on variant")
        {
            REQUIRE(computeHashOf(TestVariant(12)) == GameHash(12));
            REQUIRE(computeHashOf(TestVariant(true)) == GameHash(2));
            REQUIRE(computeHashOf(TestVariant(1.0f)) == GameHash(65538));
        }
    }

    TEST_CASE("combineHashes")
    {
        SECTION("combines hashes")
        {
            auto hash = combineHashes(GameHash(5), GameHash(6));
            REQUIRE(hash == GameHash(11));
        }

        SECTION("hashes arbitrary items before combining")
        {
            auto hash = combineHashes(true, 25, GameHash(4));
            REQUIRE(hash == GameHash(30));
        }
    }
}
