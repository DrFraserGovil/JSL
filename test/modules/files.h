#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include "../test_utils/catch_extended.h"
#include "../../modules/FileIO/FileIO.h"
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

    SECTION("initialiseFile and writeStringToFile")
    {
        fs::path p = sandbox / "string_test.txt";
        
        JSL::initialiseFile(p);
        CHECK(fs::exists(p));
        CHECK(fs::file_size(p) == 0);

        JSL::writeStringToFile(p, "Line 1\n");
        JSL::writeStringToFile(p, "Line 2", std::ios::app);
        
        CHECK(readFile(p) == "Line 1\nLine 2");
    }

    SECTION("writeVectorToFile: Variadic and Delimiters")
    {
        fs::path p = sandbox / "vec_test.csv";
        std::vector<int> v1 = {1, 2};
        std::vector<std::string> v2 = {"A", "B"};
        std::vector<double> v3 = {1.1, 2.2};

        JSL::writeVectorToFile(p, ",", v1, v2, v3);

        // Expected format:
        // 1,A,1.1
        // 2,B,2.2
        std::string content = readFile(p);
        CHECK(content.find("1,A,1.1\n") != std::string::npos);
        CHECK(content.find("2,B,2.2\n") != std::string::npos);
    }

    SECTION("writeMatrixToFile")
    {
        fs::path p = sandbox / "matrix_test.txt";
        std::vector<std::vector<int>> matrix = {{1, 2}, {3, 4}};

        // Using custom delimiters
        JSL::writeMatrixToFile(p, "|", matrix, ";");

        // Expected: 1|2;3|4;
        CHECK(readFile(p) == "1|2;3|4;");
    }

    SECTION("writeHeatMapToFile: Gnuplot Format")
    {
        fs::path p = sandbox / "heatmap.dat";
        std::vector<int> x = {10, 20};
        std::vector<int> y = {1, 2};
        std::vector<std::vector<double>> z = {{0.5, 0.6}, {0.7, 0.8}};

        JSL::writeHeatMapToFile(p, " ", x, y, z);

        // Gnuplot format expects:
        // x[0] y[0] z[0][0]
        // x[0] y[1] z[1][0]
        // (blank line)
        // x[1] y[0] z[0][1] ...
        std::string content = readFile(p);
        
        // Verify x[0] block
        CHECK(content.find("10 1 0.5\n10 2 0.7\n\n") != std::string::npos);
        // Verify x[1] block
        CHECK(content.find("20 1 0.6\n20 2 0.8\n\n") != std::string::npos);
    }

    fs::remove_all(sandbox);
}