#include <catch2/catch_test_macros.hpp>
#include "../test_utils/catch_extended.h"

#define JSL_TEST_SPOOF_PIPE
#include <JSL/FileIO.h>
#include "dummy_file.h"

// TEST_CASE("Piped Input test","[pipe][input]")
// {
//     std::stringstream fake_input("hello\nworld");
//     auto* old_buffer = std::cin.rdbuf(fake_input.rdbuf());

//     std::vector<std::string> expected = {"hello","world"};
//     int i = 0;
//     JSL::Input::forLineInPipe([&](std::string_view line) {
//         REQUIRE(expected[i] == std::string(line));
//         ++i;
//     });

//     std::cin.rdbuf(old_buffer); // Restore original buffer
// }

namespace fs = std::filesystem;
// Helper to create a dummy file for testing

TEST_CASE("Path and Directory Utilities", "[io][system]")
{
    // 1. Setup a unique sandbox for this test run
    fs::path sandbox = fs::temp_directory_path() / "JSL_Path_Tests";
    fs::remove_all(sandbox); // Start fresh
	

    SECTION("mkdir: Creation and Recursion")
    {
        fs::path deepPath = sandbox / "level1" / "level2" / "level3";
        
        // Test initial creation
        auto result = JSL::IO::mkdir(deepPath);
        REQUIRE(result.Successful);
        REQUIRE(fs::exists(deepPath));
        REQUIRE(fs::is_directory(deepPath));

        // Test calling it again (should fail with default policy)
        auto resultRepeat = JSL::IO::mkdir(deepPath);
        REQUIRE_FALSE(resultRepeat.Successful);
        
        // Test calling it again (should fail with default policy)
        resultRepeat = JSL::IO::mkdir(deepPath,JSL::IO::Quiet);
        REQUIRE(resultRepeat.Successful);
    }

    SECTION("mkdir: Failure when path is a file")
    {
		fs::create_directories(sandbox);
        fs::path filePath = sandbox / "conflict.txt";
       
		create_dummy_file(filePath);
        // Try to mkdir where a file already exists
        auto result = JSL::IO::mkdir(filePath);
        REQUIRE_FALSE(result.Successful);
    }

    // 2. Cleanup after tests
    fs::remove_all(sandbox);
}

TEST_CASE("File Deletion Utilities", "[io][system]")
{
    fs::path sandbox = fs::temp_directory_path() / "JSL_RM_Tests";
    fs::remove_all(sandbox);
    fs::create_directories(sandbox);

    SECTION("rm: Successful file removal")
    {
        fs::path target = sandbox / "delete_me.txt";
        {
            std::ofstream ofs(target);
            ofs << "gone soon";
        }

        auto result = JSL::IO::remove(target);
        CHECK(result.Successful);
        CHECK_FALSE(fs::exists(target));
        
    }

    SECTION("rm: Non-existent path handling")
    {
        fs::path ghost = sandbox / "not_there.txt";
        auto result = JSL::IO::remove(ghost,JSL::IO::Quiet);

        // Should be successful (nothing to do) but note it didn't exist
        CHECK(result.Successful);
    }

    SECTION("rm: Directory safety check")
    {
        fs::path folder = sandbox / "safe_folder";
        JSL::IO::mkdir(folder);

        // Try to delete directory without recursive flag
        auto result = JSL::IO::remove(folder);
        CHECK_FALSE(result.Successful);
        CHECK(fs::exists(folder));
    }

    SECTION("rm: Recursive directory removal")
    {
        fs::path folder = sandbox / "nest";
        fs::create_directories(folder / "inner" / "deep");
        
        // Add a file inside
        {
            std::ofstream ofs(folder / "inner" / "deep" / "data.txt");
            ofs << "payload";
        }

        auto result = JSL::IO::removeDirectory(folder);
        CHECK(result.Successful);
        CHECK_FALSE(fs::exists(folder));
        // Verify the message reports multiple items (folder + inner + deep + data.txt)
    }

    fs::remove_all(sandbox);
}