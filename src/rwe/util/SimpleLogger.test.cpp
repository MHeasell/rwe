#include <catch2/catch.hpp>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <rwe/util/SimpleLogger.h>
#include <string>
#include <vector>

namespace rwe
{
    static std::vector<std::string> readLines(const std::string& path)
    {
        std::ifstream f(path);
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(f, line))
        {
            lines.push_back(line);
        }
        return lines;
    }

    // Strip the timestamp prefix "[YYYY-MM-DD HH:MM:SS.mmm] " from a log line
    // so we can assert on the rest deterministically.
    static std::string stripTimestamp(const std::string& line)
    {
        auto pos = line.find("] ");
        if (pos == std::string::npos)
        {
            return line;
        }
        return line.substr(pos + 2);
    }

    static void initTestLogger(const std::string& path)
    {
        auto logger = std::make_shared<SimpleLogger>(path, true);
        setGlobalLogger(logger);
    }

    TEST_CASE("SimpleLogger")
    {
        const std::string path = (std::filesystem::temp_directory_path() / "rwe_test_logger.log").string();

        SECTION("writes log lines via macros")
        {
            initTestLogger(path);

            LOG_INFO << "Hello world";
            LOG_DEBUG << "Value is " << 42;
            LOG_ERROR << "Error in " << "module" << ": " << "bad thing";
            LOG_WARN << "No args here";

            // flush by replacing global logger
            setGlobalLogger(nullptr);

            auto lines = readLines(path);
            REQUIRE(lines.size() == 4);
            REQUIRE(stripTimestamp(lines[0]) == "[info] Hello world");
            REQUIRE(stripTimestamp(lines[1]) == "[debug] Value is 42");
            REQUIRE(stripTimestamp(lines[2]) == "[error] Error in module: bad thing");
            REQUIRE(stripTimestamp(lines[3]) == "[warn] No args here");

            std::remove(path.c_str());
        }

        SECTION("truncate mode overwrites existing file")
        {
            initTestLogger(path);
            LOG_INFO << "first run";
            setGlobalLogger(nullptr);

            initTestLogger(path);
            LOG_INFO << "second run";
            setGlobalLogger(nullptr);

            auto lines = readLines(path);
            REQUIRE(lines.size() == 1);
            REQUIRE(stripTimestamp(lines[0]) == "[info] second run");

            std::remove(path.c_str());
        }

        SECTION("append mode preserves existing content")
        {
            {
                auto logger = std::make_shared<SimpleLogger>(path, true);
                setGlobalLogger(logger);
                LOG_INFO << "first run";
            }
            {
                auto logger = std::make_shared<SimpleLogger>(path, false);
                setGlobalLogger(logger);
                LOG_INFO << "second run";
            }
            setGlobalLogger(nullptr);

            auto lines = readLines(path);
            REQUIRE(lines.size() == 2);
            REQUIRE(stripTimestamp(lines[0]) == "[info] first run");
            REQUIRE(stripTimestamp(lines[1]) == "[info] second run");

            std::remove(path.c_str());
        }

        SECTION("handles multiple stream insertions")
        {
            initTestLogger(path);
            LOG_INFO << "alpha" << " then " << "beta";
            setGlobalLogger(nullptr);

            auto lines = readLines(path);
            REQUIRE(lines.size() == 1);
            REQUIRE(stripTimestamp(lines[0]) == "[info] alpha then beta");

            std::remove(path.c_str());
        }

        SECTION("handles utf-8 content")
        {
            initTestLogger(path);
            LOG_INFO << "Japanese: \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e";
            LOG_INFO << "Emoji: \xf0\x9f\x8e\xae\xf0\x9f\x8e\xaf";
            LOG_INFO << "Mixed: hello \xc3\xa9\xc3\xa0\xc3\xbc world";
            LOG_INFO << "Format with utf-8: " << "user" << " said \xc2\xab" << "\xc3\xa7" "a marche\xc2\xbb";
            setGlobalLogger(nullptr);

            auto lines = readLines(path);
            REQUIRE(lines.size() == 4);
            REQUIRE(stripTimestamp(lines[0]) == "[info] Japanese: \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e");
            REQUIRE(stripTimestamp(lines[1]) == "[info] Emoji: \xf0\x9f\x8e\xae\xf0\x9f\x8e\xaf");
            REQUIRE(stripTimestamp(lines[2]) == "[info] Mixed: hello \xc3\xa9\xc3\xa0\xc3\xbc world");
            REQUIRE(stripTimestamp(lines[3]) == "[info] Format with utf-8: user said \xc2\xab\xc3\xa7" "a marche\xc2\xbb");

            std::remove(path.c_str());
        }

        SECTION("timestamp format is correct")
        {
            initTestLogger(path);
            LOG_INFO << "check timestamp";
            setGlobalLogger(nullptr);

            auto lines = readLines(path);
            REQUIRE(lines.size() == 1);
            // Verify timestamp matches [YYYY-MM-DD HH:MM:SS.mmm] pattern
            REQUIRE(lines[0][0] == '[');
            REQUIRE(lines[0][5] == '-');
            REQUIRE(lines[0][8] == '-');
            REQUIRE(lines[0][11] == ' ');
            REQUIRE(lines[0][14] == ':');
            REQUIRE(lines[0][17] == ':');
            REQUIRE(lines[0][20] == '.');
            REQUIRE(lines[0][24] == ']');

            std::remove(path.c_str());
        }

        SECTION("logs numeric types correctly")
        {
            initTestLogger(path);
            LOG_INFO << "int=" << 42 << " float=" << 3.14 << " negative=" << -1;
            setGlobalLogger(nullptr);

            auto lines = readLines(path);
            REQUIRE(lines.size() == 1);
            REQUIRE(stripTimestamp(lines[0]) == "[info] int=42 float=3.14 negative=-1");

            std::remove(path.c_str());
        }
    }
}
