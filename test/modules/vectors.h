#include <catch2/catch_test_macros.hpp>
#include <JSL/Vectors/Search.h>


TEST_CASE("Find","[find][vector][search]")
{
    SECTION("Standard Integer Search")
    {
        std::vector<int> vec = {1,2,5,10};

        REQUIRE(JSL::Vector::find(vec, 1) == 0);
        REQUIRE(JSL::Vector::find(vec,2) ==1);
        REQUIRE(JSL::Vector::find(vec,10) == 3);
        REQUIRE(!JSL::Vector::find(vec,-125));
    }

    SECTION("Empty Vector")
    {
        std::vector<int> emptyVec;
        REQUIRE(JSL::Vector::find(emptyVec,10).Found == false);
    }

    SECTION("Duplicate Values")
    {
        // Should return the index of the FIRST occurrence
        std::vector<int> duplicates = {7, 8, 7, 9};
        REQUIRE(JSL::Vector::find(duplicates,7) == 0);
        REQUIRE(JSL::Vector::find(duplicates,9) == 3);
    }

    SECTION("Different Types (Template Check)")
    {
        std::vector<std::string> strVec = {"apple", "banana", "cherry"};
        REQUIRE(JSL::Vector::find(strVec,std::string("banana")) == 1);
        REQUIRE(!JSL::Vector::find(strVec,std::string("date")));
    }
}

TEST_CASE("Find (within tolerance)","[find][vector][search]")
{
    SECTION("Standard Fuzzy-Capture")
    {
        std::vector<double> vec = {1.1,-2.2,1.09};
        REQUIRE(JSL::Vector::find(vec,1.09+1e-10,1e-8).Index == 2);
        REQUIRE(JSL::Vector::find(vec,1.09+1e-10,1e-11).Found == false);
        REQUIRE(JSL::Vector::find(vec,1.09,0.02).Index == 0);
    }

    SECTION("Empty Vector")
    {
        std::vector<double> emptyVec;
        REQUIRE(JSL::Vector::find(emptyVec,10.0,1e-1).Found == false);
    }
}