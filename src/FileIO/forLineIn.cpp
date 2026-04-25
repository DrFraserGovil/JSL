#include <JSL/FileIO/forLineIn.h>
#include <fstream>
#include <string_view>
#include <JSL/Strings.h>
#include <JSL/internal/error.h>
namespace JSL
{
    void forLineIn(const std::filesystem::path fileName, std::function<void(std::string_view)> lineProcessor)
    {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            internal::FatalError("Could not open file") << "Could not find the file '" << fileName << "'.\nPlease provide a valid filepath.";
        }

        std::string fileLine;
        while (std::getline(file, fileLine))
        {
            lineProcessor(fileLine);
        }
        file.close();
    }

    void forSplitLineIn(const std::filesystem::path fileName, std::string_view delimiter,  std::function<void(std::vector<std::string_view>)> vectorProcessor) 
    {
        forLineIn(fileName,
            [&](auto line)
            {
                vectorProcessor(split_view(line,delimiter));
            }
        );
    }
}