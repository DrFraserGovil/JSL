#include <JSL/FileIO/GetFile.h>
#include <fstream>
#include <JSL/internal/error.h>
namespace JSL::IO
{
	std::string getFile(const std::filesystem::path fileName)
	{
		std::ifstream file(fileName);
		if (!file.is_open()) {
			internal::FatalError("Could not open file", JSL_LOCATION)  << "Could not find the file '" << fileName << "'.\nPlease provide a valid filepath.";
		}
		return std::string(std::istreambuf_iterator<char>(file), {});
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