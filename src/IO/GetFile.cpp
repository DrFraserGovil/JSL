#include <JSL/IO/GetFile.h>
#include <JSL/internal/error.h>
#include <fstream>
namespace JSL::IO
{
	std::string getFile(const std::filesystem::path fileName)
	{
		std::ifstream file(fileName);
		if (!file.is_open())
		{
			internal::LibraryError("Could not open file", JSL_LOCATION) << "Could not find the file '" << fileName << "'.\nPlease provide a valid filepath.";
		}
		return std::string(std::istreambuf_iterator<char>(file), {});
	}
	std::vector<std::string> getFileLines(const std::filesystem::path fileName)
	{
		std::vector<std::string> out;
		forLineIn(fileName, [&](std::string_view line) {
			out.emplace_back(line);
		});
		return out;
	}

	inline bool is_blank(std::string_view s)
	{
		return s.find_first_not_of(" \t\n\v\f\r") == std::string_view::npos;
	}
	std::vector<std::string> getFileLines(const std::filesystem::path fileName, size_t nLines, bool includeBlank)
	{
		std::vector<std::string> out;
		bool earlyStop = false;
		forLineIn(fileName, [&](std::string_view line) {
			bool addLine = true;
			if (!includeBlank)
			{
				if (line.empty() || is_blank(line))
				{
					addLine = false;
				}
			}
			if (addLine)
			{
				out.emplace_back(line);
				earlyStop = out.size() == nLines;
			} }, earlyStop);
		return out;
	}

	std::vector<std::vector<std::string>> getFileWords(const std::filesystem::path fileName, std::string_view delimiter)
	{
		std::vector<std::vector<std::string>> out;
		forSplitLineIn(fileName, delimiter, [&](auto words) {
			out.emplace_back(words.begin(), words.end());
		});
		return out;
	}
} // namespace JSL::IO
