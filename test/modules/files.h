#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include "../test_utils/catch_extended.h"
#include "../../modules/FileIO/FileIO.h"

TEST_CASE("Reads files correctly","[file][io][input]")
{
	MockFile F;
	int linesWritten = 100;
	for (int i = 0; i < linesWritten; ++i)
	{
		F << "test\n";
	}

	int linesRead = 0;
	JSL::forLineIn(F.Path,
		[&](const std::string line)
		{
			REQUIRE(line == "test");
			linesRead++;
		}
	);
	REQUIRE(linesRead == linesWritten);
}


void GenerateVectorFile(MockFile & file, std::string delimiter, int linesWritten)
{
	for (int i =0 ; i < linesWritten; ++i)
	{
		file << i;
		file << delimiter;
		file << "test";
		file << delimiter;
		file << "gumbo!\n";
	}
}


TEST_CASE("Reads vector input correctly","[file][io][input]")
{
	
	MockFile F;
	int linesWritten = 100;

	SECTION("Simple delimiter")
	{
		int i = 0;
		GenerateVectorFile(F,",",linesWritten);
		JSL::forSplitLineIn(F.Path,",",[&](auto vec){
			REQUIRE(JSL::ParseTo<int>(vec[0])==i);
			++i;
		});
	}

	SECTION("Complex delimiter")
	{
		int i = 0;
		GenerateVectorFile(F,"_-_",linesWritten);
		JSL::forSplitLineIn(F.Path,"_-_",[&](auto vec){
			REQUIRE(JSL::ParseTo<int>(vec[0])==i);
			++i;
		});
	}

	SECTION("Tuple Reader")
	{
		int i = 0;
		GenerateVectorFile(F,",",linesWritten);
		JSL::forLineTupleIn<int,std::string,std::string>(F.Path,",",[&](auto vec){
			// REQUIRE(JSL::ParseTo<int>(vec[0])==i);
			REQUIRE(std::get<0>(vec) == i);
			REQUIRE(std::get<1>(vec) == "test");
			REQUIRE(std::get<2>(vec) == "gumbo!");
			++i;
		});
	}

	SECTION("Vector parse")
	{
		std::string s = "0,1,2,3,4,5,6";
		auto vec = JSL::ParseTo<std::vector<int>>(s);
		for (int i =0; i < 7; ++i)
		{
			REQUIRE(vec[i] == i);
		}
	}
	SECTION("Vector parse, different delimiters")
	{
		std::string s = "0_1_2_3_4_5_6";
		auto vec = JSL::ParseTo<std::vector<int>>(s,"_");
		for (int i =0; i < 7; ++i)
		{
			REQUIRE(vec[i] == i);
		}
	}
}
namespace fs = std::filesystem;
// Helper to create a dummy file for testing
void create_dummy_file(const fs::path& p)
{
    std::ofstream os(p);
    os << "data";
}

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

TEST_CASE("File System Utilities: listFiles and Globbing", "[files][io][system]")
{
    // Set up a sandbox environment
    fs::path sandbox = fs::temp_directory_path() / "JSL_test_sandbox";
    fs::remove_all(sandbox); // Clean start
    fs::create_directories(sandbox / "subdir");

    create_dummy_file(sandbox / "test1.txt");
    create_dummy_file(sandbox / "test2.log");
    create_dummy_file(sandbox / "README.md");
    create_dummy_file(sandbox / "subdir/inner.txt");
    create_dummy_file(sandbox / "subdir/notes.log");

	SECTION("Non-recursive listing")
    {
        auto results = JSL::listFiles(sandbox, false);
        
        // Should find test1, test2, README, and subdir (4 items total)
        // Note: we don't assume order, so we REQUIRE size and presence
        REQUIRE(results.size() == 4);
        
        bool foundSubdir = false;
        for (const auto& entry : results)
        {
            if (entry.Path.filename() == "subdir") 
            {
                foundSubdir = true;
                REQUIRE(entry.Status.isDirectory());
            }
        }
        REQUIRE(foundSubdir);
    }

	SECTION("Recursive listing")
    {
        auto results = JSL::listFiles(sandbox, true);
        
        // 4 items from root + 2 from subdir = 6
        REQUIRE(results.size() == 6);
    }

	SECTION("Globbing with wildcards (*.txt)")
    {
        // Non-recursive glob
        auto txtFiles = JSL::listFiles(sandbox, "*.tXt", false);
        REQUIRE(txtFiles.size() == 1);
        REQUIRE(txtFiles[0].Path.filename() == "test1.txt");

        // Recursive glob
        auto allTxtFiles = JSL::listFiles(sandbox, "*.txt", true);
        REQUIRE(allTxtFiles.size() == 2);
    }
	SECTION("Globbing with single character (?)")
    {
        auto match = JSL::listFiles(sandbox, "test?.txt", false);
        REQUIRE(match.size() == 1);
        REQUIRE(match[0].Path.filename() == "test1.txt");
    }

    SECTION("Case Insensitivity")
    {
        auto match = JSL::listFiles(sandbox, "readme.*", false);
        REQUIRE(match.size() == 1);
        REQUIRE(match[0].Path.filename() == "README.md");
    }

    SECTION("Invalid Directory Handling")
    {
        auto results = JSL::listFiles(sandbox / "non_existent_path", false);
        REQUIRE(results.empty());
    }

    // 2. Cleanup
    fs::remove_all(sandbox);
}