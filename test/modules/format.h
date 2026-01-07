#include <catch2/catch_test_macros.hpp>
#include "../../modules/Display/Display.h"

#include <chrono>
#include <thread>



TEST_CASE("Colour Testing","[display][colour]")
{
    namespace txt = JSL::Text;
    namespace bg = JSL::Background;
    namespace crs = JSL::Cursor;
    
    std::vector<std::string_view> cols = {txt::Black,txt::Red,txt::Green,txt::Yellow,txt::Blue, txt::Purple,txt::Cyan,txt::White};
    SECTION("Foreground Colours")
    {
        for (size_t i = 0; i < cols.size(); ++i)
        {
            std::string expected = "\033[3"+std::to_string(i) + "m";
            assert(cols[i] == expected);
        }
    }

    std::vector<std::string_view> backgrounds = {bg::Black,bg::Red,bg::Green,bg::Yellow,bg::Blue, bg::Purple,bg::Cyan,bg::White};
    SECTION("Background Colours")
    {
        for (size_t i = 0; i < backgrounds.size(); ++i)
        {
            std::string expected = "\033[4"+std::to_string(i) + "m";
            assert(backgrounds[i] == expected);
        }
    }

    SECTION("Colour Generation Sweep")
    {
        for (int i = 0; i < 100; ++i)
        {
            uint8_t r = rand() % 256;
            uint8_t g = rand() % 256;
            uint8_t b = rand() % 256;
            uint8_t rb = rand() % 256;
            uint8_t gb = rand() % 256;
            uint8_t bb = rand() % 256;
            REQUIRE_NOTHROW(std::cout << txt::Colour(r,g,b) << bg::Colour(rb,gb,bb) << "Testing colours"<<txt::Reset);
            std::cout << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            REQUIRE_NOTHROW(std::cout << crs::ClearLine);
            
        }
        std::cout << "\n";
    }

    SECTION("RGB Byte Accuracy")
    {
        auto check_rgb = [](uint8_t r, uint8_t g, uint8_t b, std::string expected)
        {
            std::ostringstream oss;
            oss << JSL::Text::Colour(r, g, b);
            REQUIRE(oss.str() == expected);
        };

        check_rgb(0, 0, 0, "\033[38;2;0;0;0m");
        check_rgb(255, 255, 255, "\033[38;2;255;255;255m");
        check_rgb(10, 100, 20, "\033[38;2;10;100;20m");
    }

}


TEST_CASE("oroig","[q]")
{
    int N = 100000;
    JSL::ProgressBar PB(N,N);
    PB.SetWidth(20);
    PB.SetAnimated(false);
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            for (int s = 0; s < N; ++s)
            {
                N*= 1.0/(3-2*N/N);
            }
            PB.Tick();
        }
    }
}