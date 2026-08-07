#include <JSL/Display.h>
#include <JSL/Strings/Wrap.h>
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

using namespace JSL::String;
using namespace JSL::Display;
// Raw ANSI SGR sequences, deliberately not routed through JSL::Display::*,
// since those helpers are TTY-aware and resolve to "" when stdout isn't a terminal
// (e.g. piped output, CI runners). Wrap/trueSize tests care about parsing
// escape sequences, not about Display's color-support detection.
const std::string ANSI_YELLOW = "\x1b[33m";
const std::string ANSI_BLACK = "\x1b[30m";
inline std::string ANSI_RGB(int r, int g, int b)
{
	return "\x1b[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
}
TEST_CASE("JSL::String::trueSize visual character calculation", "[trueSize]")
{
	SECTION("Calculates length of unformatted text")
	{
		REQUIRE(JSL::String::trueSize("Hello World", 4) == 11);
	}

	SECTION("Ignores ANSI sequence bytes from display color helpers")
	{
		std::string yellowText = ANSI_YELLOW + "Yellow" + ANSI_BLACK + "Black";
		REQUIRE(JSL::String::trueSize(yellowText, 4) == 11);

		std::string rgbText = ANSI_RGB(255, 128, 0) + "RGB Text";
		REQUIRE(JSL::String::trueSize(rgbText, 4) == 8);
	}

	SECTION("Expands tabs according to terminal tab size")
	{
		REQUIRE(JSL::String::trueSize("\tHi", 4) == 6);
	}
	SECTION("Wraps multiple words preserving word order and content")
	{
		std::string input = "Alpha Beta Gamma Delta";
		auto lines = JSL::String::wrap(input, 11);
		REQUIRE(lines[0] == "Alpha Beta ");
		REQUIRE(lines[1] == "Gamma Delta"); // + padding, check exact string
	}
}

TEST_CASE("wrap line wrapping and space padding", "[wrap]")
{
	SECTION("Pads short lines to full width")
	{
		std::string input = "Hello";
		auto lines = wrap(input, 10);

		REQUIRE(lines.size() == 1);
		REQUIRE(lines[0] == "Hello     ");
		REQUIRE(lines[0].size() == 10);
	}

	SECTION("Wraps multiple words while preserving exact line width")
	{
		std::string input = "Alpha Beta Gamma Delta";
		auto lines = wrap(input, 10);

		for (const auto &line : lines)
		{
			REQUIRE(JSL::String::trueSize(line, 4) == 10);
		}
	}
}

TEST_CASE("wrap oversized word splitting", "[wrap]")
{
	SECTION("Splits long word without ANSI formatting")
	{
		std::string longWord = "Supercalifragilistic";
		auto lines = wrap(longWord, 5);

		REQUIRE(lines.size() == 4);
		for (const auto &line : lines)
		{
			REQUIRE(JSL::String::trueSize(line, 4) == 5);
		}
	}

	SECTION("Splits long word containing mid-word ANSI color changes")
	{
		std::string coloredWord = "Super" + ANSI_YELLOW + "califragilistic" + ANSI_BLACK;
		auto lines = wrap(coloredWord, 5);

		REQUIRE(lines.size() == 4);
		for (const auto &line : lines)
		{
			REQUIRE(JSL::String::trueSize(line, 4) == 5);
		}
	}
}

TEST_CASE("wrap zero-visible-width and trailing ANSI sequence flushing", "[wrap]")
{
	SECTION("Appends trailing ANSI sequence to previous line without creating phantom rows")
	{
		std::string input = "Hello " + ANSI_YELLOW;
		auto lines = wrap(input, 10);

		REQUIRE(lines.size() == 1);
		REQUIRE(JSL::String::trueSize(lines[0], 4) == 10);
		REQUIRE(lines[0].find(ANSI_YELLOW) != std::string::npos);
	}

	SECTION("Handles input containing exclusively zero-width ANSI sequences")
	{
		std::string ansiOnly = ANSI_RGB(0, 255, 128);
		auto lines = wrap(ansiOnly, 10);

		REQUIRE(lines.size() == 1);
		REQUIRE(lines[0] == ansiOnly);
		REQUIRE(JSL::String::trueSize(lines[0], 4) == 0);
	}
}
TEST_CASE("ANSI sequence exactly on the split boundary", "[wrap][edge_case]")
{
	// Color sequence sits immediately between the 5th and 6th character
	std::string input = "12345" + ANSI_YELLOW + "67890";
	auto lines = wrap(input, 5);

	REQUIRE(lines.size() == 2);
	REQUIRE(trueSize(lines[0], 4) == 5);
	REQUIRE(trueSize(lines[1], 4) == 5);

	// First line should contain the first 5 chars, second line receives the color code and remaining 5 chars
	REQUIRE(lines[0] == "12345");
	REQUIRE(lines[1] == ANSI_YELLOW + "67890");
}

TEST_CASE("Tab size calculation with preceding and interleaved ANSI sequences", "[trueSize][edge_case]")
{
	SECTION("Tab following an ANSI sequence calculates offset based on visible characters")
	{
		// 2 visible chars, then ANSI, then a tab with tabSize = 4 -> tab adds 2 spaces
		std::string input = "AB" + ANSI_RGB(100, 150, 200) + "\tC";
		REQUIRE(trueSize(input, 4) == 5); // 2 ('AB') + 2 (tab offset to 4) + 1 ('C') = 5
	}

	SECTION("Tab expanding across line boundary with ANSI color state")
	{
		// "AB" (2) + tab to 4 (adds 2) + "C" (1) = 5 (fills line width 5)
		std::string input = "AB" + ANSI_BLACK + "\tC " + ANSI_YELLOW + "DEF";
		auto lines = wrap(input, 5);

		REQUIRE(lines.size() == 2);
		REQUIRE(trueSize(lines[0], 4) == 5);
		REQUIRE(trueSize(lines[1], 4) == 5);
	}
}

TEST_CASE("Dense interleaved ANSI codes inside a single oversized word", "[wrap][edge_case]")
{
	// Every single character is wrapped in a different color call
	std::string input = ANSI_BLACK + "A" +
						ANSI_YELLOW + "B" +
						ANSI_RGB(10, 20, 30) + "C" +
						ANSI_BLACK + "D";

	// Splitting at width 2
	auto lines = wrap(input, 2);

	REQUIRE(lines.size() == 2);
	REQUIRE(trueSize(lines[0], 4) == 2);
	REQUIRE(trueSize(lines[1], 4) == 2);

	// Verify ANSI sequences were preserved inline across the word boundary
	REQUIRE(lines[0] == ANSI_BLACK + "A" + ANSI_YELLOW + "B");
	REQUIRE(lines[1] == ANSI_RGB(10, 20, 30) + "C" + ANSI_BLACK + "D");
}

TEST_CASE("Multiple consecutive spaces exceeding line width", "[wrap][edge_case]")
{
	// 12 spaces between two words when width is 5
	std::string input = "A" + std::string(12, ' ') + "B";
	auto lines = wrap(input, 5);

	// Line 1: 'A' + 4 spaces
	// Remaining spaces soft-wrapped or dropped by word termination rules
	// Final line: 'B' padded to width 5
	REQUIRE(lines.front() == "A    ");
	REQUIRE(lines.back() == "B    ");

	for (const auto &line : lines)
	{
		REQUIRE(trueSize(line, 4) == 5);
	}
}
TEST_CASE("Multiple back-to-back zero-width ANSI sequences before wrapped word", "[wrap][edge_case]")
{
	// A word wrap occurs right after 3 stacked ANSI color changes
	std::string stackedANSI = ANSI_BLACK + ANSI_YELLOW + ANSI_RGB(255, 0, 0);
	std::string input = "Alpha " + stackedANSI + "Beta";

	auto lines = wrap(input, 5);

	REQUIRE(lines.size() == 2);
	REQUIRE(lines[0] == "Alpha");
	// The stacked zero-width sequences must attach to "Beta" without creating an empty row
	REQUIRE(lines[1] == stackedANSI + "Beta ");
	REQUIRE(trueSize(lines[1], 4) == 5);
}
