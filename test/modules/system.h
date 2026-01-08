#include <catch2/catch_test_macros.hpp>
#include "../test_utils/catch_extended.h"

#define JSL_TEST_SPOOF_PIPE
#include "../../modules/FileIO/pipedInput.h"


TEST_CASE("Piped Input test","[pipe][input]")
{
    std::stringstream fake_input("hello\nworld");
    auto* old_buffer = std::cin.rdbuf(fake_input.rdbuf());

    std::vector<std::string> expected = {"hello","world"};
    int i = 0;
    JSL::forLineInPipedInput([&](std::string_view line) {
        REQUIRE(expected[i] == std::string(line));
        ++i;
    });

    std::cin.rdbuf(old_buffer); // Restore original buffer
}