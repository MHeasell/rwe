#include <catch2/catch.hpp>
#include <rwe/util/SpanStream.h>
#include <string>

namespace rwe
{
    TEST_CASE("SpanStream")
    {
        const char data[] = "Hello, world!";
        auto size = sizeof(data) - 1; // exclude null terminator

        SECTION("reads data sequentially")
        {
            SpanStream stream(data, size);
            char buf[5];
            stream.read(buf, 5);
            REQUIRE(!stream.fail());
            REQUIRE(std::string(buf, 5) == "Hello");
        }

        SECTION("reports failure when reading past end")
        {
            SpanStream stream(data, size);
            char buf[20];
            stream.read(buf, 20);
            REQUIRE(stream.fail());
        }

        SECTION("tellg reports current position")
        {
            SpanStream stream(data, size);
            REQUIRE(stream.tellg() == 0);
            char buf[5];
            stream.read(buf, 5);
            REQUIRE(stream.tellg() == 5);
        }

        SECTION("seekg to absolute position")
        {
            SpanStream stream(data, size);
            stream.seekg(7);
            REQUIRE(!stream.fail());
            char buf[5];
            stream.read(buf, 5);
            REQUIRE(!stream.fail());
            REQUIRE(std::string(buf, 5) == "world");
        }

        SECTION("seekg relative to current position")
        {
            SpanStream stream(data, size);
            char buf[5];
            stream.read(buf, 5);
            stream.seekg(2, std::ios_base::cur);
            REQUIRE(!stream.fail());
            stream.read(buf, 5);
            REQUIRE(!stream.fail());
            REQUIRE(std::string(buf, 5) == "world");
        }

        SECTION("seekg relative to end")
        {
            SpanStream stream(data, size);
            stream.seekg(-6, std::ios_base::end);
            REQUIRE(!stream.fail());
            char buf[5];
            stream.read(buf, 5);
            REQUIRE(!stream.fail());
            REQUIRE(std::string(buf, 5) == "orld!");
        }

        SECTION("seekg to beginning after reading")
        {
            SpanStream stream(data, size);
            char buf[5];
            stream.read(buf, 5);
            stream.seekg(0);
            REQUIRE(!stream.fail());
            stream.read(buf, 5);
            REQUIRE(std::string(buf, 5) == "Hello");
        }

        SECTION("seekg past end fails")
        {
            SpanStream stream(data, size);
            stream.seekg(100);
            REQUIRE(stream.fail());
        }

        SECTION("seekg before beginning fails")
        {
            SpanStream stream(data, size);
            stream.seekg(-1);
            REQUIRE(stream.fail());
        }

        SECTION("works with binary data containing null bytes")
        {
            const char binData[] = {'\x00', '\x01', '\x02', '\x03', '\x04'};
            SpanStream stream(binData, 5);
            stream.seekg(2);
            char val;
            stream.read(&val, 1);
            REQUIRE(!stream.fail());
            REQUIRE(val == '\x02');
        }
    }
}
