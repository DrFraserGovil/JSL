#include <catch2/catch_test_macros.hpp>
#include "../test_utils/catch_extended.h"

#define JSL_TEST_SPOOF_PIPE
#include "../../modules/FileIO/FileIO.h"
#include "dummy_file.h"

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

namespace fs = std::filesystem;
// Helper to create a dummy file for testing

TEST_CASE("Path and Directory Utilities", "[io][system]")
{
    // 1. Setup a unique sandbox for this test run
    fs::path sandbox = fs::temp_directory_path() / "JSL_Path_Tests";
    fs::remove_all(sandbox); // Start fresh
	

    SECTION("pathStatus: Correctly identifies types")
    {
        // Test non-existent
        auto statusNone = JSL::pathStatus(sandbox.string());
        REQUIRE_FALSE(statusNone.exists);
        REQUIRE(statusNone.type == JSL::PathType::NotFound);

        // Create directory and test
        fs::create_directories(sandbox);
        auto statusDir = JSL::pathStatus(sandbox.string());
        REQUIRE(statusDir.exists);
        REQUIRE(statusDir.isDirectory());
        REQUIRE_FALSE(statusDir.isFile());

        // Create file and test
        fs::path filePath = sandbox / "test.txt";
        {
            std::ofstream ofs(filePath);
            ofs << "dummy data";
        }
        auto statusFile = JSL::pathStatus(filePath.string());
        REQUIRE(statusFile.exists);
        REQUIRE(statusFile.isFile());
        REQUIRE(statusFile.type == JSL::PathType::File);
    }

    SECTION("mkdir: Creation and Recursion")
    {
        fs::path deepPath = sandbox / "level1" / "level2" / "level3";
        
        // Test initial creation
        auto result = JSL::mkdir(deepPath);
        REQUIRE(result.Successful);
        REQUIRE(fs::exists(deepPath));
        REQUIRE(fs::is_directory(deepPath));

        // Test calling it again (should ignore and be successful)
        auto resultRepeat = JSL::mkdir(deepPath);
        REQUIRE(resultRepeat.Successful);
        REQUIRE(resultRepeat.Message.find("already exists") != std::string::npos);
    }

    SECTION("mkdir: Failure when path is a file")
    {
		fs::create_directories(sandbox);
        fs::path filePath = sandbox / "conflict.txt";
       
		create_dummy_file(filePath);
        // Try to mkdir where a file already exists
        auto result = JSL::mkdir(filePath);
        REQUIRE_FALSE(result.Successful);
        REQUIRE(result.Message.find("ERROR") != std::string::npos);
        REQUIRE(result.Message.find("is not a directory") != std::string::npos);
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

        auto result = JSL::rm(target);
        CHECK(result.Successful);
        CHECK_FALSE(fs::exists(target));
        CHECK(result.Message.find("Successfully removed") != std::string::npos);
    }

    SECTION("rm: Non-existent path handling")
    {
        fs::path ghost = sandbox / "not_there.txt";
        auto result = JSL::rm(ghost);

        // Should be successful (nothing to do) but note it didn't exist
        CHECK(result.Successful);
        CHECK(result.Message.find("Path does not exist") != std::string::npos);
    }

    SECTION("rm: Directory safety check")
    {
        fs::path folder = sandbox / "safe_folder";
        JSL::mkdir(folder);

        // Try to delete directory without recursive flag
        auto result = JSL::rm(folder, false);
        CHECK_FALSE(result.Successful);
        CHECK(fs::exists(folder));
        CHECK(result.Message.find("without recursive flag") != std::string::npos);
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

        auto result = JSL::rm(folder, true);
        CHECK(result.Successful);
        CHECK_FALSE(fs::exists(folder));
        // Verify the message reports multiple items (folder + inner + deep + data.txt)
        CHECK(result.Message.find("Successfully removed") != std::string::npos);
    }

    fs::remove_all(sandbox);
}