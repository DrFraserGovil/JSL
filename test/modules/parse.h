#pragma once
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <string_view>
#include "../../modules/Strings/ParseTo.h"


TEST_CASE("Edge case handling","[utility][ParseTo][edgecase]")
{

	SECTION("Check errors")
	{
		std::string errorMessage = capture_stdout([&](){
			REQUIRE_THROWS(JSL::ParseTo<int>(""));
		});
		REQUIRE_THAT(errorMessage,ContainsSubstring("Library Error"));
		REQUIRE_THAT(errorMessage,ContainsSubstring("empty"));

		errorMessage = capture_stdout([&](){
			REQUIRE_THROWS(JSL::ParseTo<int>("1.5"));
		});
		REQUIRE_THAT(errorMessage,ContainsSubstring("Library Error"));
		REQUIRE_THAT(errorMessage,ContainsSubstring("Partial conversion"));


		errorMessage = capture_stdout([&](){
			REQUIRE_THROWS(JSL::ParseTo<int>("99999999999"));
		});
		REQUIRE_THAT(errorMessage,ContainsSubstring("Library Error"));
		REQUIRE_THAT(errorMessage,ContainsSubstring("out of range"));


		errorMessage = capture_stdout([&](){
			REQUIRE_THROWS(JSL::ParseTo<int>("true"));
		});
		REQUIRE_THAT(errorMessage,ContainsSubstring("Library Error"));
		REQUIRE_THAT(errorMessage,ContainsSubstring("Invalid argument"));

		errorMessage = capture_stdout([&](){
			REQUIRE_THROWS(JSL::ParseTo<int>("-"));
		});
		REQUIRE_THAT(errorMessage,ContainsSubstring("Library Error"));
		REQUIRE_THAT(errorMessage,ContainsSubstring("Invalid argument"));

		
	}

	SECTION("Consistent whitespace trimming")
	{
		REQUIRE_NOTHROW(JSL::ParseTo<int>("1 "));
		REQUIRE_NOTHROW(JSL::ParseTo<bool>("1 "));
		REQUIRE_NOTHROW(JSL::ParseTo<double>("1 "));
		REQUIRE_NOTHROW(JSL::ParseTo<std::vector<int>>("1 "));

		//check that string does *not* trim!s
		REQUIRE(JSL::ParseTo<std::string>(" hi") == " hi");
	}
}

TEST_CASE("Basic types","[utility][JSL::ParseTo]")
{
	SECTION("Integral types")
	{
		// Basic Valid Cases
        REQUIRE(JSL::ParseTo<int>("1") == 1);
        REQUIRE(JSL::ParseTo<int>("-1") == -1);
        REQUIRE(JSL::ParseTo<int>("101") == 101);
        REQUIRE(JSL::ParseTo<unsigned int>("123") == 123u);
        REQUIRE(JSL::ParseTo<long>("12345678901") == 12345678901L);
        REQUIRE(JSL::ParseTo<long long>("999999999991") == 999999999991LL);
	}

	SECTION("doubles")
	{
		REQUIRE(JSL::ParseTo<double>("15") == 15.0);
        REQUIRE(JSL::ParseTo<double>("1.5") == 1.5);
        REQUIRE(JSL::ParseTo<double>("-1.5") == -1.5);
        REQUIRE(JSL::ParseTo<float>("3.14") == 3.14f);
        REQUIRE(JSL::ParseTo<double>("0.0") == 0.0);
        REQUIRE(JSL::ParseTo<double>(".5") == 0.5);
        REQUIRE(JSL::ParseTo<double>("1.") == 1.0);

		 // Scientific Notation
		 REQUIRE(JSL::ParseTo<double>("1e3") == 1000.0);
		 REQUIRE(JSL::ParseTo<double>("1.23e-5") == 0.0000123);
		 REQUIRE(JSL::ParseTo<double>("-2.5E+2") == -250.0); // Test uppercase E
 
		 // Special Values (std::stod/stof handle these)
		 REQUIRE(std::isinf(JSL::ParseTo<double>("inf")));
		 REQUIRE(std::isinf(JSL::ParseTo<double>("-inf")));
		 REQUIRE(std::isnan(JSL::ParseTo<double>("nan")));
		 REQUIRE(std::isinf(JSL::ParseTo<float>("INF"))); // std::stod/stof are case-insensitive for inf/nan
 
	}

	SECTION("Booleans")
	{
		REQUIRE(JSL::ParseTo<bool>("1") == true );
		REQUIRE(JSL::ParseTo<bool>("TRUE") == true );
		REQUIRE(JSL::ParseTo<bool>("TRUE") == true );
		REQUIRE(JSL::ParseTo<bool>("FAlse") == false );
		REQUIRE(JSL::ParseTo<bool>("0") == false );
		auto errorMessage = capture_stdout([&](){
			REQUIRE_THROWS(JSL::ParseTo<bool>("2"));
		});
		REQUIRE_THAT(errorMessage,ContainsSubstring("Library Error"));
	}
}

TEST_CASE("Vector-conversion","[utility][JSL::ParseTo][vector]")
{
	SECTION("Vector -ints")
	{
		// Basic Valid Cases
        REQUIRE(JSL::ParseTo<std::vector<int>>("1,2,3") == std::vector<int>{1, 2, 3});
        REQUIRE(JSL::ParseTo<std::vector<int>>("[1,2,3]") == std::vector<int>{1, 2, 3});
        REQUIRE(JSL::ParseTo<std::vector<int>>("{1,2,3}") == std::vector<int>{1, 2, 3});
        REQUIRE(JSL::ParseTo<std::vector<int>>("(1,2,3)") == std::vector<int>{1, 2, 3});

        // With custom delimiter
        REQUIRE(JSL::ParseTo<std::vector<int>>("1;2;3", ";") == std::vector<int>{1, 2, 3});
        REQUIRE(JSL::ParseTo<std::vector<int>>("[1;2;3]", ";") == std::vector<int>{1, 2, 3});

        // With whitespace in elements (requires `trim` in vector loop for non-floating types)
        REQUIRE(JSL::ParseTo<std::vector<int>>(" 1 , 2 , 3 ") == std::vector<int>{1, 2, 3});
        REQUIRE(JSL::ParseTo<std::vector<int>>("[ 1 , 2 , 3 ]") == std::vector<int>{1, 2, 3});
        REQUIRE(JSL::ParseTo<std::vector<int>>(" 1 ; 2 ; 3 ", ";") == std::vector<int>{1, 2, 3});

        // Empty vector representations
        REQUIRE(JSL::ParseTo<std::vector<int>>("[]") == std::vector<int>{});
        REQUIRE(JSL::ParseTo<std::vector<int>>("{}") == std::vector<int>{});
        REQUIRE(JSL::ParseTo<std::vector<int>>("()") == std::vector<int>{});

		// Error Cases
		std::string errorMessage;

		// Empty input string to vector (throws due to vector specialization's RejectEmpty)
		errorMessage = capture_stdout([&]() {
			REQUIRE_THROWS_AS(JSL::ParseTo<std::vector<int>>(""), std::runtime_error);
		});
		REQUIRE_THAT(errorMessage, ContainsSubstring("Library Error"));
		REQUIRE_THAT(errorMessage, ContainsSubstring("empty string"));

		// Malformed elements (inner conversion failure)
		errorMessage = capture_stdout([&]() {
			REQUIRE_THROWS_AS(JSL::ParseTo<std::vector<int>>("1,abc,3"), std::runtime_error);
		});
		REQUIRE_THAT(errorMessage, ContainsSubstring("Library Error"));
		REQUIRE_THAT(errorMessage, ContainsSubstring("Invalid argument"));

		// Empty elements (e.g. "1,,3")
		errorMessage = capture_stdout([&]() {
			REQUIRE_THROWS_AS(JSL::ParseTo<std::vector<int>>("1,,3"), std::runtime_error);
		});
		REQUIRE_THAT(errorMessage, ContainsSubstring("Library Error"));
		REQUIRE_THAT(errorMessage, ContainsSubstring("empty")); // From inner int conversion's RejectEmpty
	}

	SECTION("Vector Conversions - Doubles")
	{
		// Basic Valid Cases
		REQUIRE(JSL::ParseTo<std::vector<double>>("1.1,2.2,3.3") == std::vector<double>{1.1, 2.2, 3.3});
		REQUIRE(JSL::ParseTo<std::vector<double>>("[1.1,2.2,3.3]") == std::vector<double>{1.1, 2.2, 3.3});

		// With whitespace in elements (should work fine due to std::stod skipping whitespace AND trim)
		REQUIRE(JSL::ParseTo<std::vector<double>>(" 1.1 , 2.2 , 3.3 ") == std::vector<double>{1.1, 2.2, 3.3});

		// Error Cases
		std::string errorMessage;

		// Malformed elements
		errorMessage = capture_stdout([&]() {
			REQUIRE_THROWS_AS(JSL::ParseTo<std::vector<double>>("1.0,invalid,3.0"), std::runtime_error);
		});
		REQUIRE_THAT(errorMessage, ContainsSubstring("Library Error"));
		REQUIRE_THAT(errorMessage, ContainsSubstring("Invalid argument"));

		// Empty elements
		errorMessage = capture_stdout([&]() {
			REQUIRE_THROWS_AS(JSL::ParseTo<std::vector<double>>("1.0,,3.0"), std::runtime_error);
		});
		REQUIRE_THAT(errorMessage, ContainsSubstring("Library Error"));
		REQUIRE_THAT(errorMessage, ContainsSubstring("empty"));
		}

		SECTION("Vector Conversions - Strings")
		{
		// Basic Valid Cases
		REQUIRE(JSL::ParseTo<std::vector<std::string>>("a,b,c") == std::vector<std::string>{"a", "b", "c"});
		REQUIRE(JSL::ParseTo<std::vector<std::string>>("[hello,world]") == std::vector<std::string>{"hello", "world"});

		// Empty string elements (valid for std::string type)
		

		// Empty input string to vector (throws due to vector specialization's RejectEmpty)
		std::string errorMessage = capture_stdout([&]() {
			REQUIRE_THROWS_AS(JSL::ParseTo<std::vector<std::string>>("a,,c"),std::runtime_error);
			REQUIRE_THROWS_AS(JSL::ParseTo<std::vector<std::string>>(",a,"),std::runtime_error);
			REQUIRE_THROWS_AS(JSL::ParseTo<std::vector<std::string>>(""), std::runtime_error);
		});
		REQUIRE_THAT(errorMessage, ContainsSubstring("Library Error"));
		REQUIRE_THAT(errorMessage, ContainsSubstring("empty string"));


		// Empty bracket representation
		REQUIRE(JSL::ParseTo<std::vector<std::string>>("[]") == std::vector<std::string>{});
	}
	


	
}