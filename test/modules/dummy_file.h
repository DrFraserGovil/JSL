#pragma once
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;
void inline create_dummy_file(const fs::path& p)
{
    std::ofstream os(p);
    os << "data";
}