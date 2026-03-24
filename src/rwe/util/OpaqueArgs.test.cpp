#include <catch2/catch_test_macros.hpp>
#include <rwe/util/OpaqueArgs.h>
#include <sstream>

namespace rwe
{
    // Helper to build argc/argv from a vector of strings.
    // The first element should be the program name.
    struct ArgBuilder
    {
        std::vector<std::string> storage;
        std::vector<char*> ptrs;

        ArgBuilder(std::initializer_list<std::string> args) : storage(args)
        {
            for (auto& s : storage)
            {
                ptrs.push_back(s.data());
            }
        }

        int argc() const { return static_cast<int>(ptrs.size()); }
        char** argv() { return ptrs.data(); }
    };

    TEST_CASE("OpaqueArgs")
    {
        SECTION("parses --key value pairs")
        {
            ArgBuilder args{"rwe", "--width", "1024", "--height", "768"};
            OpaqueArgs oa;
            oa.parse(args.argc(), args.argv());
            REQUIRE(oa.getUint("width", 800) == 1024);
            REQUIRE(oa.getUint("height", 600) == 768);
        }

        SECTION("parses --key=value pairs")
        {
            ArgBuilder args{"rwe", "--width=1024", "--height=768"};
            OpaqueArgs oa;
            oa.parse(args.argc(), args.argv());
            REQUIRE(oa.getUint("width", 800) == 1024);
            REQUIRE(oa.getUint("height", 600) == 768);
        }

        SECTION("returns defaults for missing keys")
        {
            ArgBuilder args{"rwe"};
            OpaqueArgs oa;
            oa.parse(args.argc(), args.argv());
            REQUIRE(oa.getUint("width", 800) == 800);
            REQUIRE(oa.getString("port", "1337") == "1337");
            REQUIRE(oa.getBool("fullscreen") == false);
        }

        SECTION("parses bool flags")
        {
            ArgBuilder args{"rwe", "--fullscreen"};
            OpaqueArgs oa;
            oa.parse(args.argc(), args.argv());
            REQUIRE(oa.getBool("fullscreen") == true);
        }

        SECTION("bool flag followed by another flag")
        {
            ArgBuilder args{"rwe", "--fullscreen", "--width", "1024"};
            OpaqueArgs oa;
            oa.parse(args.argc(), args.argv());
            REQUIRE(oa.getBool("fullscreen") == true);
            REQUIRE(oa.getUint("width", 800) == 1024);
        }

        SECTION("multi-value args accumulate")
        {
            ArgBuilder args{"rwe", "--data-path", "/foo", "--data-path", "/bar"};
            OpaqueArgs oa;
            oa.parse(args.argc(), args.argv());
            auto paths = oa.getMulti("data-path");
            REQUIRE(paths.size() == 2);
            REQUIRE(paths[0] == "/foo");
            REQUIRE(paths[1] == "/bar");
        }

        SECTION("--help sets help flag")
        {
            ArgBuilder args{"rwe", "--help"};
            OpaqueArgs oa;
            oa.parse(args.argc(), args.argv());
            REQUIRE(oa.isHelpRequested());
        }

        SECTION("getString throws on missing required key")
        {
            ArgBuilder args{"rwe"};
            OpaqueArgs oa;
            oa.parse(args.argc(), args.argv());
            REQUIRE_THROWS_AS(oa.getString("map"), std::runtime_error);
        }

        SECTION("contains returns true for present keys")
        {
            ArgBuilder args{"rwe", "--map", "coast"};
            OpaqueArgs oa;
            oa.parse(args.argc(), args.argv());
            REQUIRE(oa.contains("map"));
            REQUIRE(!oa.contains("log"));
        }

        SECTION("throws on unexpected argument without --")
        {
            ArgBuilder args{"rwe", "bogus"};
            OpaqueArgs oa;
            REQUIRE_THROWS_AS(oa.parse(args.argc(), args.argv()), std::runtime_error);
        }

        SECTION("parses config file")
        {
            std::istringstream config(
                "width=1024\n"
                "height=768\n"
                "fullscreen=true\n");

            OpaqueArgs oa;
            ArgBuilder args{"rwe"};
            oa.parse(args.argc(), args.argv());
            oa.parseConfig(config);
            REQUIRE(oa.getUint("width", 800) == 1024);
            REQUIRE(oa.getUint("height", 600) == 768);
            REQUIRE(oa.getBool("fullscreen") == true);
        }

        SECTION("config file skips comments and blank lines")
        {
            std::istringstream config(
                "# this is a comment\n"
                "\n"
                "width=1024\n"
                "   \n"
                "# another comment\n");

            OpaqueArgs oa;
            ArgBuilder args{"rwe"};
            oa.parse(args.argc(), args.argv());
            oa.parseConfig(config);
            REQUIRE(oa.getUint("width", 800) == 1024);
        }

        SECTION("command line takes precedence over config file")
        {
            std::istringstream config("width=640\nheight=480\n");

            ArgBuilder args{"rwe", "--width", "1024"};
            OpaqueArgs oa;
            oa.parse(args.argc(), args.argv());
            oa.parseConfig(config);
            REQUIRE(oa.getUint("width", 800) == 1024);
            REQUIRE(oa.getUint("height", 600) == 480);
        }

        SECTION("config file trims whitespace around key and value")
        {
            std::istringstream config("  width = 1024  \n");

            OpaqueArgs oa;
            ArgBuilder args{"rwe"};
            oa.parse(args.argc(), args.argv());
            oa.parseConfig(config);
            REQUIRE(oa.getUint("width", 800) == 1024);
        }

        SECTION("config file throws on line without =")
        {
            std::istringstream config("badline\n");

            OpaqueArgs oa;
            ArgBuilder args{"rwe"};
            oa.parse(args.argc(), args.argv());
            REQUIRE_THROWS_AS(oa.parseConfig(config), std::runtime_error);
        }

        SECTION("player args accumulate via multi")
        {
            ArgBuilder args{"rwe", "--map", "coast",
                "--player", "Player1;human;arm;blue",
                "--player", "Player2;computer;core;red"};
            OpaqueArgs oa;
            oa.parse(args.argc(), args.argv());
            auto players = oa.getMulti("player");
            REQUIRE(players.size() == 2);
            REQUIRE(players[0] == "Player1;human;arm;blue");
            REQUIRE(players[1] == "Player2;computer;core;red");
        }

        SECTION("getString with default returns last value for multi-value key")
        {
            ArgBuilder args{"rwe", "--interface-mode", "left-click", "--interface-mode", "right-click"};
            OpaqueArgs oa;
            oa.parse(args.argc(), args.argv());
            REQUIRE(oa.getString("interface-mode", "left-click") == "right-click");
        }

        SECTION("all dir-* options work")
        {
            ArgBuilder args{"rwe", "--dir-ai", "myai", "--dir-maps", "mymaps"};
            OpaqueArgs oa;
            oa.parse(args.argc(), args.argv());
            REQUIRE(oa.getString("dir-ai", "ai") == "myai");
            REQUIRE(oa.getString("dir-maps", "maps") == "mymaps");
            REQUIRE(oa.getString("dir-sounds", "sounds") == "sounds");
        }
    }
}
