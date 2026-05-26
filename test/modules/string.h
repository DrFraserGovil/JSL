#pragma once
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <JSL/Strings.h>
TEST_CASE("Case-insensitive-equality testing","[string]")
{
    REQUIRE(JSL::String::iEquals("HELLO","Hello"));
    REQUIRE(JSL::String::iEquals("HELLO miXing AND matching","Hello miXing anD mAtCHINg"));
    REQUIRE(!JSL::String::iEquals("HELLO","Hello miXing anD mAtCHINg"));
}

TEST_CASE("String Split functionality", "[JSL][string][split]")
{
    using namespace JSL::String;

    SECTION("Standard splitting")
    {
        auto result = split_view("one,two,three", ",");
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == "one");
        REQUIRE(result[1] == "two");
        REQUIRE(result[2] == "three");
    }

    SECTION("Multi-character delimiters")
    {
        auto result = split_view("A::B::C", "::");
        REQUIRE(result.size() == 3);
        REQUIRE(result[1] == "B");
    }

    SECTION("Empty elements and trailing delimiters")
    {
        auto result = split_view(",,val,", ",");
        // Expect: "", "", "val", ""
        REQUIRE(result.size() == 4);
        REQUIRE(result[0].empty());
        REQUIRE(result[2] == "val");
        REQUIRE(result[3].empty());
    }

    SECTION("No delimiter found")
    {
        auto result = split_view("hello", ",");
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == "hello");
    }

    SECTION("Empty delimiter error handling")
    {
        REQUIRE_THROWS_AS(split_view("test", ""), std::runtime_error);
    }
}

TEST_CASE("String Trim functionality", "[JSL][string][trim]")
{
    using namespace JSL::String;

    SECTION("Basic whitespace trimming")
    {
        REQUIRE(trim_view("  hello  ") == "hello");
        REQUIRE(trim_view("\t\n  tabs and lines \r") == "tabs and lines");
        REQUIRE(trim_view("no_spaces") == "no_spaces");
    }

    SECTION("Empty or whitespace-only strings")
    {
        REQUIRE(trim_view("").empty());
        REQUIRE(trim_view("    ").empty());
    }

    SECTION("Trimming with comment indicators")
    {
        std::string indicator = "//";
        
        // Basic comment stripping
        REQUIRE(trim_view("data // comment", indicator) == "data");
        
        // Comment at start
        REQUIRE(trim_view("// hidden line", indicator).empty());
        
        // Comment with no whitespace
        REQUIRE(trim_view("value//comment", indicator) == "value");
    }

    SECTION("Comment indicator not present")
    {
        REQUIRE(trim_view("just data", "#") == "just data");
    }
}