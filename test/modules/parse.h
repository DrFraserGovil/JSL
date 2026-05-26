#pragma once
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <string>
#include <string_view>
#include <JSL/Display.h>
#include <JSL/Strings/ParseTo.h>
#include <cmath>
using namespace Catch::Matchers;
TEST_CASE("Edge case handling","[utility][ParseTo][edgecase]")
{

	SECTION("Check errors")
	{
		REQUIRE_THROWS(JSL::String::ParseTo<int>(""));
		REQUIRE_THROWS(JSL::String::ParseTo<int>("1.5"));
		REQUIRE_THROWS(JSL::String::ParseTo<int>("99999999999"));
		REQUIRE_THROWS(JSL::String::ParseTo<int>("true"));
		REQUIRE_THROWS(JSL::String::ParseTo<int>("-"));
	}

	SECTION("Consistent whitespace trimming")
	{
		REQUIRE_NOTHROW(JSL::String::ParseTo<int>("1 "));
		REQUIRE_NOTHROW(JSL::String::ParseTo<bool>("1 "));
		REQUIRE_NOTHROW(JSL::String::ParseTo<double>("1 "));
		REQUIRE_NOTHROW(JSL::String::ParseTo<std::vector<int>>("1 "));

		REQUIRE(JSL::String::ParseTo<std::string>(" hi\t") == "hi");
	}
}

TEST_CASE("Basic types","[utility][JSL::String::ParseTo]")
{
	SECTION("Integral types")
	{
		// Basic Valid Cases
        REQUIRE(JSL::String::ParseTo<int>("1") == 1);
        REQUIRE(JSL::String::ParseTo<int>("-1") == -1);
        REQUIRE(JSL::String::ParseTo<int>("101") == 101);
        REQUIRE(JSL::String::ParseTo<unsigned int>("123") == 123u);
        REQUIRE(JSL::String::ParseTo<long>("12345678901") == 12345678901L);
        REQUIRE(JSL::String::ParseTo<long long>("999999999991") == 999999999991LL);
	}

	SECTION("doubles")
	{
		REQUIRE(JSL::String::ParseTo<double>("15") == 15.0);
        REQUIRE(JSL::String::ParseTo<double>("1.5") == 1.5);
        REQUIRE(JSL::String::ParseTo<double>("-1.5") == -1.5);
        REQUIRE(JSL::String::ParseTo<float>("3.14") == 3.14f);
        REQUIRE(JSL::String::ParseTo<double>("0.0") == 0.0);
        REQUIRE(JSL::String::ParseTo<double>(".5") == 0.5);
        REQUIRE(JSL::String::ParseTo<double>("1.") == 1.0);

		 // Scientific Notation
		 REQUIRE(JSL::String::ParseTo<double>("1e3") == 1000.0);
		 REQUIRE(JSL::String::ParseTo<double>("1.23e-5") == 0.0000123);
		 REQUIRE(JSL::String::ParseTo<double>("-2.5E+2") == -250.0); // Test uppercase E
 
		 // Special Values (std::stod/stof handle these)
		 REQUIRE(std::isinf(JSL::String::ParseTo<double>("inf")));
		 REQUIRE(std::isinf(JSL::String::ParseTo<double>("-inf")));
		 REQUIRE(std::isnan(JSL::String::ParseTo<double>("nan")));
		 REQUIRE(std::isinf(JSL::String::ParseTo<float>("INF"))); // std::stod/stof are case-insensitive for inf/nan
 
	}

	SECTION("Booleans")
	{
		REQUIRE(JSL::String::ParseTo<bool>("1") == true );
		REQUIRE(JSL::String::ParseTo<bool>("TRUE") == true );
		REQUIRE(JSL::String::ParseTo<bool>("TRUE") == true );
		REQUIRE(JSL::String::ParseTo<bool>("FAlse") == false );
		REQUIRE(JSL::String::ParseTo<bool>("0") == false);
		REQUIRE_THROWS(JSL::String::ParseTo<bool>("2"));
	}
}

TEST_CASE("Vector-conversion","[utility][JSL::String::ParseTo][vector]")
{
	SECTION("Vector -ints")
	{
		// Basic Valid Cases
        REQUIRE(JSL::String::ParseTo<std::vector<int>>("1,2,3") == std::vector<int>{1, 2, 3});
        REQUIRE(JSL::String::ParseTo<std::vector<int>>("[1,2,3]") == std::vector<int>{1, 2, 3});
        REQUIRE(JSL::String::ParseTo<std::vector<int>>("{1,2,3}") == std::vector<int>{1, 2, 3});
        REQUIRE(JSL::String::ParseTo<std::vector<int>>("(1,2,3)") == std::vector<int>{1, 2, 3});

        // With custom delimiter
        REQUIRE(JSL::String::ParseTo<std::vector<int>>("1;2;3", ";") == std::vector<int>{1, 2, 3});
        REQUIRE(JSL::String::ParseTo<std::vector<int>>("[1;2;3]", ";") == std::vector<int>{1, 2, 3});

        // With whitespace in elements (requires `trim` in vector loop for non-floating types)
        REQUIRE(JSL::String::ParseTo<std::vector<int>>(" 1 , 2 , 3 ") == std::vector<int>{1, 2, 3});
        REQUIRE(JSL::String::ParseTo<std::vector<int>>("[ 1 , 2 , 3 ]") == std::vector<int>{1, 2, 3});
        REQUIRE(JSL::String::ParseTo<std::vector<int>>(" 1 ; 2 ; 3 ", ";") == std::vector<int>{1, 2, 3});

        // Empty vector representations
        REQUIRE(JSL::String::ParseTo<std::vector<int>>("[]") == std::vector<int>{});
        REQUIRE(JSL::String::ParseTo<std::vector<int>>("{}") == std::vector<int>{});
        REQUIRE(JSL::String::ParseTo<std::vector<int>>("()") == std::vector<int>{});

		// Error Cases

		// Empty input string to vector (throws due to vector specialization's RejectEmpty)
		REQUIRE_THROWS_AS(JSL::String::ParseTo<std::vector<int>>(""), std::runtime_error);
		REQUIRE_THROWS_AS(JSL::String::ParseTo<std::vector<int>>("1,abc,3"), std::runtime_error);

	}

	SECTION("Vector Conversions - Doubles")
	{
		// Basic Valid Cases
		REQUIRE(JSL::String::ParseTo<std::vector<double>>("1.1,2.2,3.3") == std::vector<double>{1.1, 2.2, 3.3});
		REQUIRE(JSL::String::ParseTo<std::vector<double>>("[1.1,2.2,3.3]") == std::vector<double>{1.1, 2.2, 3.3});

		// With whitespace in elements (should work fine due to std::stod skipping whitespace AND trim)
		REQUIRE(JSL::String::ParseTo<std::vector<double>>(" 1.1 , 2.2 , 3.3 ") == std::vector<double>{1.1, 2.2, 3.3});

		// Error Cases
		std::string errorMessage;

		// Malformed elements
		REQUIRE_THROWS_AS(JSL::String::ParseTo<std::vector<double>>("1.0,invalid,3.0"), std::runtime_error);

	}

	SECTION("Vector Conversions - Strings")
	{
		// Basic Valid Cases
		REQUIRE(JSL::String::ParseTo<std::vector<std::string>>("a,b,c") == std::vector<std::string>{"a", "b", "c"});
		REQUIRE(JSL::String::ParseTo<std::vector<std::string>>("[hello,world]") == std::vector<std::string>{"hello", "world"});

		// Empty string elements (valid for std::string type)
		

		// Empty bracket representation
		REQUIRE(JSL::String::ParseTo<std::vector<std::string>>("[]") == std::vector<std::string>{});
	}
	


	
}