#pragma once
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include "../test_utils/catch_extended.h"
#include "JSL/Strings/MakeFrom.h"
#include <JSL/IO/Vault.h>
#include <filesystem>

using namespace JSL::IO;

TEST_CASE("Vault Writing", "[archive][write]")
{
    MockFile file;
    VaultWriter A(file.Name());



    SECTION("Basic writing using operator[]")
    {
        // Vault creates the .part file immediately
        REQUIRE(std::filesystem::exists(file.Name() + ".part")); 
       
        //Manually open the file
        REQUIRE_NOTHROW(A.NewFile("test-file"));
        REQUIRE_NOTHROW(A["test-file"] << "some data");
    }
    
    SECTION("Strictness checks")
    {
        //Defaults to strictmode, so should fail here...
        REQUIRE_THROWS(A["test-file"] << "some data");
        
        A.SetPolicy(JSL::IO::Policy::Generous);
        REQUIRE_NOTHROW(A["test-file"] << "some data");
    }
    A.SetPolicy(Policy::Generous);
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
        VaultWriter A; 
        REQUIRE_THROWS(A["badfile"] << "data");
        
    }

    SECTION("Corrupted Archive Detection")
    {
        // Setup a non-TAR file
        {
            std::ofstream f(file.Name());
            f << "This is just a random text file, not a TAR";
        }
        
            // BuildIndex() should fail the null-termination check
            REQUIRE_THROWS(VaultReader(file.Name()));
        
    }
}

TEST_CASE("Vault Reading", "[archive][read]")
{
    MockFile file;
    {
        VaultWriter W(file.Name(),Policy::Generous);
        for (int i = 0; i < 5; ++i)
        {
            W["test-file-" + std::to_string(i+1)] << "Test_data_" << (10*i+5);
        }
        W.Close();
    }

    VaultReader R(file.Name());

    SECTION("File Listing")
    {
        auto filelist = R.Files();
        REQUIRE(filelist.size() == 5);
        REQUIRE(filelist.contains("test-file-1"));
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
        VaultWriter W(file.Name(),Policy::Generous);
        auto& s = W["table.dat"];
        for (int i = 0; i < 5; ++i)
        {
            s << i << " " << (double)i * 1.5 << " " << (char)('A' + i) << "\n";
        }
        W.Close();
    }

    VaultReader R(file.Name());

    SECTION("AsTable conversion")
    {
        // Using the new variadic template method
        auto table = R["table.dat"].AsTable<int, double, char>(" ");
        
        REQUIRE(table.size() == 5);
        REQUIRE(std::get<0>(table[2]) == 2);
        REQUIRE(std::get<1>(table[2]) == 3.0);
        REQUIRE(std::get<2>(table[2]) == 'C');
    }

    SECTION("ForConvertedLineIn callback")
    {
        int count = 0;
        R.ForConvertedLineIn<int, double, char>("table.dat", " ", [&](auto a, auto b, auto c) {
            REQUIRE(a == count);
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
        VaultWriter W(file.Name(),Policy::Generous);
        W.NewFile("big_payload.bin",true);
        W << largeData; // Direct streaming to disk
        W["t2"] << "hi";
        W.Close();
    }

    // Verify with standard reader
    VaultReader R(file.Name());
    REQUIRE(R["big_payload.bin"].AsText() == largeData);
}