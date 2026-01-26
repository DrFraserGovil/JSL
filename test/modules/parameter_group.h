#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#define SETTINGS_FILE "settings.def"
#include "../../modules/Parameters/Group.h"
#include "../../modules/FileIO/FileIO.h"


TEST_CASE("Settings Object Functionality", "[settings][macro]") 
{
    SECTION("Default values are correctly initialized") 
    {
        JSL::Settings s;
        REQUIRE(s.iterations == 10);
        REQUIRE(s.factor == 1.5);
        REQUIRE(s.flag == false);
        REQUIRE(s.counts == std::vector<int>{1, 2, 3});
    }

    SECTION("Command line arguments override defaults") 
    {
        // Simulate: ./app -i 42 -b -c 10,20
        char* argv[] = {(char*)"test", (char*)"-i", (char*)"42", (char*)"-b", (char*)"-c", (char*)"10,20"};
        int argc = 6;

        JSL::Settings s(argc, argv);
        REQUIRE(s.iterations == 42);
        REQUIRE(s.flag == true);
        REQUIRE(s.counts == std::vector<int>{10, 20});
        REQUIRE(s.factor == 1.5); // Remains default
    }

    SECTION("Configuration file overrides defaults") 
    {
        // Create a temporary config file
        std::string cfg_name = "test_config.txt";
        std::ofstream ofs(cfg_name);
        ofs << "i 100\n";
        ofs << "f 2.718\n";
        ofs.close();

        JSL::Settings s;
        s.Configure(cfg_name, " ");

        REQUIRE(s.iterations == 100);
        REQUIRE_THAT(s.factor,Catch::Matchers::WithinAbs(2.718,1e-10));
        JSL::rm("test_config.txt");

    }

    SECTION("CLI overrides both defaults and config file") 
    {
        std::ofstream ofs("layered.txt");
        ofs << "i 500\n";
        ofs.close();

        // CLI says -i 999, which should beat the file's 500
        char* argv[] = {(char*)"test", (char*)"--config", (char*)"layered.txt", (char*)"-i", (char*)"999"};
        int argc = 5;

        JSL::Settings s(argc, argv);
        REQUIRE(s.iterations == 999);
          JSL::rm("layered.txt");
    }
}