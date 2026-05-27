#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include "../test_utils/catch_extended.h"
#include <JSL/FileIO.h>
#include <JSL.h>
#include "dummy_file.h"
TEST_CASE("Reads files correctly","[file][io][input]")
{
	MockFile F;
	int linesWritten = 100;
	for (int i = 0; i < linesWritten; ++i)
	{
		F << "test\n";
	}

	int linesRead = 0;
	JSL::IO::forLineIn(F.Path,
		[&](auto line)
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
		JSL::IO::forSplitLineIn(F.Path,",",[&](auto vec){
			REQUIRE(JSL::String::ParseTo<int>(vec[0])==i);
			++i;
		});
	}

	SECTION("Complex delimiter")
	{
		int i = 0;
		GenerateVectorFile(F,"_-_",linesWritten);
		JSL::IO::forSplitLineIn(F.Path,"_-_",[&](auto vec){
			REQUIRE(JSL::String::ParseTo<int>(vec[0])==i);
			++i;
		});
	}

	SECTION("Tuple Reader")
	{
		int i = 0;
		GenerateVectorFile(F,",",linesWritten);
		JSL::IO::forConvertedLineIn<int,std::string,std::string>(F.Path,",",[&](int a, std::string b, std::string c){
			// REQUIRE(JSL::String::ParseTo<int>(vec[0])==i);
			REQUIRE(a == i);
			REQUIRE(b == "test");
			REQUIRE(c == "gumbo!");
			++i;
		});
	}

	SECTION("Vector parse")
	{
		std::string s = "0,1,2,3,4,5,6";
		auto vec = JSL::String::ParseTo<std::vector<int>>(s);
		for (int i =0; i < 7; ++i)
		{
			REQUIRE(vec[i] == i);
		}
	}
	SECTION("Vector parse, different delimiters")
	{
		std::string s = "0_1_2_3_4_5_6";
		auto vec = JSL::String::ParseTo<std::vector<int>>(s,"_");
		for (int i =0; i < 7; ++i)
		{
			REQUIRE(vec[i] == i);
		}
	}
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
        auto results = JSL::IO::Directory::list(sandbox, false);
        
        // Should find test1, test2, README, and subdir (4 items total)
        // Note: we don't assume order, so we REQUIRE size and presence
        REQUIRE(results.size() == 4);
        
        bool foundSubdir = false;
        for (const auto& entry : results)
        {
            if (entry.filename() == "subdir") 
            {
                foundSubdir = true;
            }
        }
        REQUIRE(foundSubdir);
    }

	SECTION("Recursive listing")
    {
        auto results = JSL::IO::Directory::list(sandbox, true);
        // 4 items from root + 2 from subdir = 6
        REQUIRE(results.size() == 6);
    }

	SECTION("Globbing with wildcards (*.txt)")
    {
        // Non-recursive glob
        auto txtFiles = JSL::IO::Directory::match(sandbox, "*.txt", false);
        REQUIRE(txtFiles.size() == 1);
        REQUIRE(txtFiles.contains(sandbox/"test1.txt"));

        // Recursive glob
        // auto q = JSL::IO::Filesystem::ListFiles
        auto allTxtFiles = JSL::IO::Directory::match(sandbox, "*.txt", true);
        REQUIRE(allTxtFiles.size() == 2);
    }
	SECTION("Globbing with single character (?)")
    {
        auto match = JSL::IO::Directory::match(sandbox, "test?.txt", false);
        REQUIRE(match.size() == 1);
        REQUIRE(match.contains(sandbox/"test1.txt"));
    }

    SECTION("Case Sensitivity")
    {
        auto match = JSL::IO::Directory::match(sandbox, "readme.*", false);
        REQUIRE(match.size() == 0);
    }

    SECTION("Invalid Directory Handling")
    {
        auto results = JSL::IO::Directory::list(sandbox / "non_existent_path", false);
        REQUIRE(results.empty());
    }

    // 2. Cleanup
    fs::remove_all(sandbox);
}

// Helper to read entire file content into a string for verification
std::string readFile(const fs::path& p)
{
    std::ifstream ifs(p);
    return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

TEST_CASE("JSL I/O Utilities", "[io]")
{
    fs::path sandbox = fs::temp_directory_path() / "JSL_IO_Tests";
    fs::remove_all(sandbox);
    fs::create_directories(sandbox);

    SECTION("initialiseFile and writeString")
    {
        fs::path p = sandbox / "string_test.txt";
        
        JSL::IO::initialise(p);
        CHECK(fs::exists(p));
        CHECK(fs::file_size(p) == 0);

        JSL::IO::writeString(p, "Line 1\n");
        JSL::IO::writeString(p, "Line 2", std::ios::app);
        
        CHECK(readFile(p) == "Line 1\nLine 2");
    }

    SECTION("writeVector: Variadic and Delimiters")
    {
        fs::path p = sandbox / "vec_test.csv";
        std::vector<int> v1 = {1, 2};
        std::vector<std::string> v2 = {"A", "B"};
        std::vector<double> v3 = {1.1, 2.2};

        JSL::IO::writeRows(p, ",", v1, v2, v3);

        // Expected format:
        // 1,A,1.1
        // 2,B,2.2
        std::string content = readFile(p);
        CHECK(content.find("1,A,1.1\n") != std::string::npos);
        CHECK(content.find("2,B,2.2\n") != std::string::npos);
    }

    SECTION("writeMatrix")
    {
        fs::path p = sandbox / "matrix_test.txt";
        std::vector<std::vector<int>> matrix = {{1, 2}, {3, 4}};

        // Using custom delimiters
        JSL::IO::writeRows(p, "|", matrix, ";");

        // Expected: 1|2;3|4;
        CHECK(readFile(p) == "1|2;3|4;");
    }

    fs::remove_all(sandbox);
}