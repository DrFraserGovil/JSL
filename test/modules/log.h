#pragma once
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
// #include <catch2/matchers/catch_matchers_contains.hpp>

#include "../test_utils/catch_extended.h"
#include <JSL/Display/Log.h>

using namespace Catch::Matchers;
TEST_CASE("Logger Core", "[log][utility]") {
    JSL::Log::Config cfg;

    SECTION("Log Configuration Object") 
	{
		SECTION("Default Construction")
		{
			//Check that the default values are as assumed    
			REQUIRE(cfg.Level == INFO);
			REQUIRE(cfg.ShowHeaders == true);
			REQUIRE(cfg.AppendNewline == true);
		}
		
		SECTION("SetLevel(int)") 
		{
			//Check the integer assignment 
			cfg.SetLevel(0); REQUIRE(cfg.Level == ERROR);
			cfg.SetLevel(1); REQUIRE(cfg.Level == WARN);
			cfg.SetLevel(2); REQUIRE(cfg.Level == INFO);
			cfg.SetLevel(3); REQUIRE(cfg.Level == DEBUG);
	
			REQUIRE_THROWS_AS(cfg.SetLevel(4), std::runtime_error);
			REQUIRE_THROWS_AS(cfg.SetLevel(-1), std::runtime_error);
		}

		SECTION("SetLevel(LogLevel)") {
			cfg.SetLevel(INFO); REQUIRE(cfg.Level == INFO);
			cfg.SetLevel(ERROR); REQUIRE(cfg.Level == ERROR);
			cfg.SetLevel(WARN); REQUIRE(cfg.Level == WARN);
			cfg.SetLevel(DEBUG); REQUIRE(cfg.Level == DEBUG);
		}
	
		
    }
	
	SECTION("Core Logger Output")
	{
		// Control the global config object for testing
		auto originalConfig = JSL::Log::Global::Config; // Save original config
		JSL::Log::Global::Config.AppendNewline = true; // Assume terminal for colored tests
		JSL::Log::Global::Config.TerminalOutput = false; // Assume terminal for colored tests
		JSL::Log::Global::Config.ShowHeaders = (false);
		
		

		SECTION("Check ANSI code insertion")
		{
			auto r = {DEBUG,INFO,WARN,ERROR};
			for (LogLevel level: r)
			{
				JSL::Log::Global::Config.TerminalOutput = true;
				std::string terminalOutput = capture_stdout([&]() {
					(JSL::Log::Core(level,0,"mock-function","mock-file")) << "Debug message";
				});
				REQUIRE_THAT(terminalOutput,StartsWith("\033[")); //check that an ANSI codes is inserted at the beginning of input. We don't care which -- that's an implementation detail that can be changed
				REQUIRE_THAT(terminalOutput, EndsWith("\n\033[0m"));  //check that the default ANSI code (white) is inserted at the end

				JSL::Log::Global::Config.TerminalOutput = false;
				std::string fileOutput = capture_stdout([&]() {
					(JSL::Log::Core(DEBUG,0,"mock-function","mock-file")) << "Debug message";
				});
				REQUIRE_THAT(fileOutput,!StartsWith("\033[")); //check the above tests fail when terminal output deactivated
				REQUIRE_THAT(fileOutput, !EndsWith("\n\033[0m"));
			}
		}
		
	
		SECTION("Check headers")
		{
			auto r = {DEBUG,INFO,WARN,ERROR};
			std::vector<std::string> names = {"DEBUG","INFO","WARN","ERROR"};
			int i = 0;
			JSL::Log::Global::Config.TerminalOutput = false;
			JSL::Log::Global::Config.AppendNewline= false;
			for (LogLevel level: r)
			{
				JSL::Log::Global::Config.ShowHeaders = (true);
				std::string headerPresent = capture_stdout([&]() {
					(JSL::Log::Core(level,0,"mock-function","mock-file")) << "Debug message";
				});
				std::string search = "[" + names[i] + "]";
				REQUIRE_THAT(headerPresent,ContainsSubstring(search));//check that the header successfully inserted when in true mode
				++i;

				JSL::Log::Global::Config.ShowHeaders = (false);
				std::string headerAbsent = capture_stdout([&]() {
					(JSL::Log::Core(level,0,"mock-function","mock-file")) << "Debug message";
				});
				REQUIRE_THAT(headerAbsent, !ContainsSubstring(search)); // check that it's absent in false mode
			}
		}

		SECTION("Empty log")
		{
			std::string output = capture_stdout([&]() {
				(JSL::Log::Core(DEBUG,0,"mock-function","mock-file"));
			});
			REQUIRE(output.empty());
		}

		JSL::Log::Global::Config = originalConfig;
	}	
}

TEST_CASE("Logger Macro","[log][utility]")
{
	JSL::Log::Global::Config.ShowHeaders = (true);
	JSL::Log::Global::Config.AppendNewline = (false); //set these so no linebreaks in unit test output. Purely for human readability.
	JSL::Log::Global::Config.TerminalOutput = false; // Also supress ANSI codes.
	std::vector<std::string> names = {"ERROR","WARN","INFO","DEBUG"};
	for (int trueLevel = 0; trueLevel < 4; ++trueLevel)
	{
		JSL::Log::Global::Config.SetLevel(trueLevel);

		for (int mockLevel = 0; mockLevel < 4; ++mockLevel)
		{
			auto mock = JSL::Log::LogLevelConvert(mockLevel);
			std::string logOutput = capture_stdout([&]() {
				LOG(mock) << "Test log" << " secondary log" << "tertiary log"; // deliberately ruined spaces
			});

			if (mockLevel > trueLevel)
			{
				REQUIRE(logOutput.empty());
			}
			else
			{
				std::string expectedHeader = "[" + names[mockLevel] + "]";
				REQUIRE_THAT(logOutput,ContainsSubstring(expectedHeader));
				std::string expectedBody = "Test log secondary logtertiary log"; //testing the deliberately ruined spaces
				REQUIRE_THAT(logOutput,ContainsSubstring(expectedBody));
			}
		}
	}

	JSL::Log::Global::Config.SetLevel(1);
}