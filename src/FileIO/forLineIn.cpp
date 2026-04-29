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
            internal::FatalError("Could not open file", JSL_LOCATION)  << "Could not find the file '" << fileName << "'.\nPlease provide a valid filepath.";
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


    std::vector<std::string> getFileLines(const std::filesystem::path fileName)
    {
        std::vector<std::string> out;
        forLineIn(fileName,[&](std::string_view line)
        {
            out.emplace_back(line);
        });
        return out;
    }
    std::vector<std::vector<std::string>> getFileWords(const std::filesystem::path fileName,std::string_view delimiter)
    {
        std::vector<std::vector<std::string>> out;
        forSplitLineIn(fileName,delimiter,[&](auto words)
        {
            out.emplace_back(words.begin(),words.end());
        });
        return out;
    }
}