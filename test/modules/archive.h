#pragma once
#include <catch2/catch_test_macros.hpp>
#include "../../modules/FileIO/archiver.h"
#include <iostream>
#include <filesystem>

using namespace JSL::Archiver;

TEST_CASE("Vault Writing", "[archive][write]")
{
    MockFile file;
    Vault<Mode::Write> A(file.Name());

    SECTION("Basic writing using operator[]")
    {
        // Vault creates the .part file immediately
        REQUIRE(std::filesystem::exists(file.Name() + ".part")); 
        
        // Use the new operator[] syntax
        REQUIRE_NOTHROW(A["test-file"] << "some data");
    }

    SECTION("Multiple files and closing")
    {
        A["file1"] << "data1";
        A["file2"] << "data2";
        A.Close();
        
        REQUIRE(std::filesystem::exists(file.Name()));
        REQUIRE_FALSE(std::filesystem::exists(file.Name() + ".part"));
    }
}

TEST_CASE("Vault Modes detect valid states", "[archive][errors]")
{
    MockFile file;
    
    SECTION("Uninitialised state protection")
    {
        Vault<Mode::Write> A; 
        std::string output = capture_stdout([&]() {
            // We expect the FatalError to print and then throw
            REQUIRE_THROWS(A["badfile"] << "data");
        });
        
        REQUIRE_THAT(output, Catch::Matchers::ContainsSubstring("JSL"));
    }

    SECTION("Corrupted Archive Detection")
    {
        // Setup a non-TAR file
        {
            std::ofstream f(file.Name());
            f << "This is just a random text file, not a TAR";
        }
        
        std::string output = capture_stdout([&]() {
            // BuildIndex() should fail the null-termination check
            REQUIRE_THROWS(Vault<Mode::Read>(file.Name()));
        });
        
        REQUIRE_THAT(output, Catch::Matchers::ContainsSubstring("JSL"));
    }
}

TEST_CASE("Vault Reading", "[archive][read]")
{
    MockFile file;
    {
        Vault<Mode::Write> W(file.Name());
        for (int i = 0; i < 5; ++i)
        {
            W["test-file-" + std::to_string(i+1)] << "Test_data_" << (10*i+5);
        }
        W.Close();
    }

    Vault<Mode::Read> R(file.Name());

    SECTION("File Listing")
    {
        auto filelist = R.ListFiles();
        REQUIRE(filelist.size() == 5);
        REQUIRE_THAT(filelist, Catch::Matchers::VectorContains((std::string)"test-file-1"));
    }

    SECTION("Extraction using AsText")
    {
        for (int i = 0; i < 5; ++i)
        {
            std::string name = "test-file-" + std::to_string(i+1);
            std::string expected = "Test_data_" + std::to_string(10*i+5);
            
            // New AsText() method on the ReadStream
            REQUIRE(R[name].AsText() == expected);
        }
    }
}

TEST_CASE("Tabular Data and Stream Logic", "[archive][tabular]")
{
    MockFile file;
    {
        Vault<Mode::Write> W(file.Name());
        auto& s = W["table.dat"];
        for (int i = 0; i < 5; ++i)
        {
            s << i << " " << (double)i * 1.5 << " " << (char)('A' + i) << "\n";
        }
        W.Close();
    }

    Vault<Mode::Read> R(file.Name());

    SECTION("AsTable conversion")
    {
        // Using the new variadic template method
        auto table = R["table.dat"].AsTable<int, double, char>(" ");
        
        REQUIRE(table.size() == 5);
        REQUIRE(std::get<0>(table[2]) == 2);
        REQUIRE(std::get<1>(table[2]) == 3.0);
        REQUIRE(std::get<2>(table[2]) == 'C');
    }

    SECTION("ForTabularLineIn callback")
    {
        int count = 0;
        R.ForTabularLineIn<int, double, char>("table.dat", " ", [&](auto row) {
            REQUIRE(std::get<0>(row) == count);
            count++;
        });
        REQUIRE(count == 5);
    }
}

TEST_CASE("Large File Seekp Logic", "[archive][largefile]")
{
    MockFile file;
    std::string largeData = "This is a significant chunk of data that we stream directly.";
    
    {
        Vault<Mode::Write> W(file.Name());
        W.SetLargeFile("big_payload.bin");
        W << largeData; // Direct streaming to disk
        W.Close();
    }

    // Verify with standard reader
    Vault<Mode::Read> R(file.Name());
    REQUIRE(R["big_payload.bin"].AsText() == largeData);
}