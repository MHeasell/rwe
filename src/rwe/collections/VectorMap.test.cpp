#include <catch2/catch.hpp>
#include <rapidcheck.h>
#include <rapidcheck/catch.h>
#include <rwe/OpaqueId.h>
#include <rwe/OpaqueId_io.h>
#include <rwe/collections/VectorMap.h>
#include <rwe/optional_io.h>
#include <rwe/rc_gen_optional.h>

namespace rwe
{
    struct IdTag;
    using Id = OpaqueId<unsigned int, IdTag>;
    TEST_CASE("VectorMap")
    {
        SECTION("can store and retrieve elements")
        {
            VectorMap<char, IdTag> m;

            auto idA = m.emplace('a');
            auto idB = m.emplace('b');
            auto idC = m.emplace('c');
            REQUIRE(idA == Id(0));
            REQUIRE(idB == Id(256));
            REQUIRE(idC == Id(512));

            REQUIRE(m.tryGet(idA) == 'a');
            REQUIRE(m.tryGet(idB) == 'b');
            REQUIRE(m.tryGet(idC) == 'c');
        }

        SECTION("works with const")
        {
            VectorMap<int, IdTag> m;
            const auto& c = m;

            auto id = m.emplace(1);

            auto elem = c.tryGet(id);
            REQUIRE(elem == 1);
        }

        SECTION("allows removing elements")
        {
            VectorMap<char, IdTag> m;
            auto idA = m.emplace('a');
            REQUIRE(m.tryGet(idA) == 'a');
            m.remove(idA);
            REQUIRE(m.tryGet(idA) == std::nullopt);
        }

        SECTION("allows removing elements and filling in holes")
        {
            VectorMap<char, IdTag> m;

            // add some elements
            auto idA = m.emplace('a');
            auto idB = m.emplace('b');
            auto idC = m.emplace('c');
            auto idD = m.emplace('d');
            auto idE = m.emplace('e');

            // delete some (make some holes)
            m.remove(idD);
            m.remove(idB);

            // add new elements to fill in the holes
            auto idF = m.emplace('f');
            auto idG = m.emplace('g');
            auto idH = m.emplace('h');

            REQUIRE(idA == Id(0));
            REQUIRE(idB == Id(256));
            REQUIRE(idC == Id(512));
            REQUIRE(idD == Id(768));
            REQUIRE(idE == Id(1024));

            REQUIRE(idF == Id(257));  // idB + 1 generation
            REQUIRE(idG == Id(769));  // idD + 1 generation
            REQUIRE(idH == Id(1280)); // new slot

            REQUIRE(m.tryGet(idA) == 'a');
            REQUIRE(m.tryGet(idB) == std::nullopt);
            REQUIRE(m.tryGet(idC) == 'c');
            REQUIRE(m.tryGet(idD) == std::nullopt);
            REQUIRE(m.tryGet(idE) == 'e');
            REQUIRE(m.tryGet(idF) == 'f');
            REQUIRE(m.tryGet(idG) == 'g');
            REQUIRE(m.tryGet(idH) == 'h');

            m.remove(idG);
            auto idI = m.emplace('i');
            REQUIRE(idI == Id(770)); // idD + 2 generations
            REQUIRE(m.tryGet(idG) == std::nullopt);
            REQUIRE(m.tryGet(idI) == 'i');
        }

        SECTION("generations wrap around to 0")
        {
            VectorMap<char, IdTag> m;
            for (int i = 0; i < 255; ++i)
            {
                auto id = m.emplace('x');
                m.remove(id);
            }

            auto id = m.emplace('a');
            REQUIRE(id == Id(255));
            m.remove(id);
            auto id2 = m.emplace('a');
            REQUIRE(id2 == Id(0));
            REQUIRE(m.tryGet(id2) == 'a');
        }

        SECTION("supports iteration")
        {
            VectorMap<char, IdTag> m;
            m.emplace('a');
            m.emplace('b');
            m.emplace('c');
            m.emplace('d');
            m.emplace('e');

            std::vector<std::pair<Id, char>> items{{Id(0), 'a'}, {Id(256), 'b'}, {Id(512), 'c'}, {Id(768), 'd'}, {Id(1024), 'e'}};
            int i = 0;
            for (const auto& p : m)
            {
                REQUIRE(p == items.at(i));
                ++i;
            }
        }

        SECTION("supports const iteration")
        {
            VectorMap<char, IdTag> m;
            m.emplace('a');
            m.emplace('b');
            m.emplace('c');
            m.emplace('d');
            m.emplace('e');

            const auto& x = m;

            std::vector<std::pair<Id, char>> items{{Id(0), 'a'}, {Id(256), 'b'}, {Id(512), 'c'}, {Id(768), 'd'}, {Id(1024), 'e'}};
            int i = 0;
            for (const auto& p : x)
            {
                REQUIRE(p == items.at(i));
                ++i;
            }
        }

        SECTION("iteration skips holes at end")
        {
            VectorMap<char, IdTag> m;
            m.emplace('a');
            m.emplace('b');
            m.emplace('c');
            auto idD = m.emplace('d');
            auto idE = m.emplace('e');

            m.remove(idD);
            m.remove(idE);

            std::vector<std::pair<Id, char>> items{{Id(0), 'a'}, {Id(256), 'b'}, {Id(512), 'c'}};
            int i = 0;
            for (const auto& p : m)
            {
                REQUIRE(p == items.at(i));
                ++i;
            }
        }

        SECTION("iteration skips holes at beginning")
        {
            VectorMap<char, IdTag> m;
            auto idA = m.emplace('a');
            auto idB = m.emplace('b');
            m.emplace('c');
            m.emplace('d');
            m.emplace('e');

            m.remove(idA);
            m.remove(idB);

            std::vector<std::pair<Id, char>> items{{Id(512), 'c'}, {Id(768), 'd'}, {Id(1024), 'e'}};
            int i = 0;
            for (const auto& p : m)
            {
                REQUIRE(p == items.at(i));
                ++i;
            }
        }

        SECTION("iteration skips holes in the middle")
        {
            VectorMap<char, IdTag> m;
            m.emplace('a');
            m.emplace('b');
            auto idC = m.emplace('c');
            m.emplace('d');
            auto idE = m.emplace('e');
            auto idF = m.emplace('f');
            m.emplace('g');

            m.remove(idC);
            m.remove(idE);
            m.remove(idF);

            std::vector<std::pair<Id, char>> items{{Id(0), 'a'}, {Id(256), 'b'}, {Id(768), 'd'}, {Id(1536), 'g'}};
            int i = 0;
            for (const auto& p : m)
            {
                REQUIRE(p == items.at(i));
                ++i;
            }
        }

        SECTION("supports erasing during iteration")
        {
            VectorMap<char, IdTag> m;
            m.emplace('a');
            m.emplace('b');
            m.emplace('c');
            m.emplace('d');
            m.emplace('e');

            auto it = std::find_if(m.begin(), m.end(), [](const auto& p) { return p.second == 'c'; });
            it = m.erase(it);

            REQUIRE(it->second == 'd');
        }

        SECTION("erasing skips holes after")
        {
            VectorMap<char, IdTag> m;
            m.emplace('a');
            m.emplace('b');
            m.emplace('c');
            auto dId = m.emplace('d');
            m.emplace('e');

            m.remove(dId);

            auto it = std::find_if(m.begin(), m.end(), [](const auto& p) { return p.second == 'c'; });
            it = m.erase(it);

            REQUIRE(it->second == 'e');
        }

        SECTION("supports find")
        {
            VectorMap<char, IdTag> m;
            m.emplace('a');
            m.emplace('b');
            m.emplace('c');
            auto dId = m.emplace('d');
            m.emplace('e');

            auto it = m.find(dId);
            REQUIRE(it->first == Id(768));
            REQUIRE(it->second == 'd');

            m.remove(dId);

            REQUIRE(m.find(dId) == m.end());
        }

        SECTION("find works with const")
        {
            VectorMap<char, IdTag> m;
            m.emplace('a');
            m.emplace('b');
            m.emplace('c');
            auto dId = m.emplace('d');
            m.emplace('e');

            const auto& c = m;

            auto it = c.find(dId);
            REQUIRE(it->first == Id(768));
            REQUIRE(it->second == 'd');

            m.remove(dId);

            REQUIRE(c.find(dId) == c.end());
        }

        rc::prop("can insert and remove elements and it doesn't break", [](std::vector<std::optional<int>> ops) {
            VectorMap<int, IdTag> m;

            std::vector<std::pair<Id, int>> ids;

            long long total = 0;
            for (const auto& o : ops)
            {
                if (o)
                {
                    total += *o;
                    auto id = m.emplace(*o);
                    ids.emplace_back(id, *o);
                }
                else
                {
                    auto idIndex = *rc::gen::inRange<std::size_t>(0, ids.size());
                    m.remove(ids[idIndex].first);
                    total -= ids[idIndex].second;
                    ids.erase(ids.begin() + idIndex);
                }

                long long currentTotal = 0;
                for (const auto& e : m)
                {
                    auto it = std::find_if(ids.begin(), ids.end(), [&e](const auto& x) { return x.first == e.first; });
                    RC_ASSERT(it != ids.end());
                    currentTotal += e.second;
                }
                RC_ASSERT(currentTotal == total);
            }
        });
    }
}
