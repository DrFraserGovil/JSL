#include <catch2/catch_test_macros.hpp>
#include <JSL/Vectors/Search.h>


TEST_CASE("Find","[find][vector][search]")
{
    SECTION("Standard Integer Search")
    {
        std::vector<int> vec = {1,2,5,10};
        
        REQUIRE(JSL::find(1,vec).Index == 0);
        REQUIRE(JSL::find(2,vec).Index ==1);
        REQUIRE(JSL::find(10,vec).Index == 3);
        REQUIRE(JSL::find(-125,vec).Found == false);
    }

    SECTION("Empty Vector")
    {
        std::vector<int> emptyVec;
        REQUIRE(JSL::find(10, emptyVec).Found == false);
    }

    SECTION("Duplicate Values")
    {
        // Should return the index of the FIRST occurrence
        std::vector<int> duplicates = {7, 8, 7, 9};
        REQUIRE(JSL::find(7, duplicates).Index == 0);
        REQUIRE(JSL::find(9, duplicates).Index == 3);
    }

    SECTION("Different Types (Template Check)")
    {
        std::vector<std::string> strVec = {"apple", "banana", "cherry"};
        REQUIRE(JSL::find(std::string("banana"), strVec).Index == 1);
        REQUIRE(JSL::find(std::string("date"), strVec).Found == false);
    }
}

TEST_CASE("Find (within tolerance)","[find][vector][search]")
{
    SECTION("Standard Fuzzy-Capture")
    {
        std::vector<double> vec = {1.1,-2.2,1.09};
        REQUIRE(JSL::find(1.09+1e-10,vec,1e-8).Index == 2);
        REQUIRE(JSL::find(1.09+1e-10,vec,1e-11).Found == false);
        REQUIRE(JSL::find(1.09,vec,0.02).Index == 0);
    }

    SECTION("Empty Vector")
    {
        std::vector<double> emptyVec;
        REQUIRE(JSL::find(10.0, emptyVec,1e-1).Found == false);
    }
}