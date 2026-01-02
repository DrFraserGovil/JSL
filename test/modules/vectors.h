#include <catch2/catch_test_macros.hpp>
#include "../../modules/Vectors/Search.h"


TEST_CASE("Find","[find][vector][search]")
{
    SECTION("Standard Integer Search")
    {
        std::vector<int> vec = {1,2,5,10};
        REQUIRE(JSL::Find(1,vec) == 0);
        REQUIRE(JSL::Find(2,vec) == 1);
        REQUIRE(JSL::Find(10,vec) == 3);
        REQUIRE(JSL::Find(-125,vec) == JSL::NotFound);
    }

    SECTION("Empty Vector")
    {
        std::vector<int> emptyVec;
        REQUIRE(JSL::Find(10, emptyVec) == JSL::NotFound);
    }

    SECTION("Duplicate Values")
    {
        // Should return the index of the FIRST occurrence
        std::vector<int> duplicates = {7, 8, 7, 9};
        REQUIRE(JSL::Find(7, duplicates) == 0);
        REQUIRE(JSL::Find(9, duplicates) == 3);
    }

    SECTION("Different Types (Template Check)")
    {
        std::vector<std::string> strVec = {"apple", "banana", "cherry"};
        REQUIRE(JSL::Find(std::string("banana"), strVec) == 1);
        REQUIRE(JSL::Find(std::string("date"), strVec) == JSL::NotFound);
    }
}

TEST_CASE("Find (within tolerance)","[find][vector][search]")
{
    SECTION("Standard Fuzzy-Capture")
    {
        std::vector<double> vec = {1.1,-2.2,1.09};
        REQUIRE(JSL::Find(1.09+1e-10,vec,1e-8) == 2);
        REQUIRE(JSL::Find(1.09+1e-10,vec,1e-11) == JSL::NotFound);
        REQUIRE(JSL::Find(1.09,vec,0.02) == 0);
    }

    SECTION("Empty Vector")
    {
        std::vector<double> emptyVec;
        REQUIRE(JSL::Find(10.0, emptyVec,1e-1) == JSL::NotFound);
    }
}