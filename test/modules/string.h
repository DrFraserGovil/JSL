#pragma once
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "../../modules/Strings/Strings.h"

TEST_CASE("Case-insensitive-equality testing","[string]")
{
    REQUIRE(JSL::insensitiveEquals("HELLO","Hello"));
    REQUIRE(JSL::insensitiveEquals("HELLO miXing AND matching","Hello miXing anD mAtCHINg"));
    REQUIRE(!JSL::insensitiveEquals("HELLO","Hello miXing anD mAtCHINg"));
}

TEST_CASE("String Split functionality", "[JSL][string][split]")
{
    using namespace JSL;

    SECTION("Standard splitting")
    {
        auto result = split("one,two,three", ",");
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == "one");
        REQUIRE(result[1] == "two");
        REQUIRE(result[2] == "three");
    }

    SECTION("Multi-character delimiters")
    {
        auto result = split("A::B::C", "::");
        REQUIRE(result.size() == 3);
        REQUIRE(result[1] == "B");
    }

    SECTION("Empty elements and trailing delimiters")
    {
        auto result = split(",,val,", ",");
        // Expect: "", "", "val", ""
        REQUIRE(result.size() == 4);
        REQUIRE(result[0].empty());
        REQUIRE(result[2] == "val");
        REQUIRE(result[3].empty());
    }

    SECTION("No delimiter found")
    {
        auto result = split("hello", ",");
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == "hello");
    }

    SECTION("Empty delimiter error handling")
    {
        REQUIRE_THROWS_AS(split("test", ""), std::runtime_error);
    }
}

TEST_CASE("String Trim functionality", "[JSL][string][trim]")
{
    using namespace JSL;

    SECTION("Basic whitespace trimming")
    {
        REQUIRE(trim("  hello  ") == "hello");
        REQUIRE(trim("\t\n  tabs and lines \r") == "tabs and lines");
        REQUIRE(trim("no_spaces") == "no_spaces");
    }

    SECTION("Empty or whitespace-only strings")
    {
        REQUIRE(trim("").empty());
        REQUIRE(trim("    ").empty());
    }

    SECTION("Trimming with comment indicators")
    {
        std::string indicator = "//";
        
        // Basic comment stripping
        REQUIRE(trim("data // comment", indicator) == "data");
        
        // Comment at start
        REQUIRE(trim("// hidden line", indicator).empty());
        
        // Comment with no whitespace
        REQUIRE(trim("value//comment", indicator) == "value");
    }

    SECTION("Comment indicator not present")
    {
        REQUIRE(trim("just data", "#") == "just data");
    }
}