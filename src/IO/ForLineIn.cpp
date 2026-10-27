#include <JSL/IO/ForLineIn.h>
#include <JSL/internal/error.h>
#include <fstream>
#include <string_view>
namespace JSL::IO
{
	void forLineIn(const std::filesystem::path fileName, std::function<void(std::string_view)> lineProcessor, bool &earlyStop)
	{
		std::ifstream file(fileName);
		if (!file.is_open())
		{
			internal::LibraryError("Could not open file", JSL_LOCATION) << "Could not find the file '" << fileName << "'.\nPlease provide a valid filepath.";
		}
		std::string fileLine;
		while (!earlyStop && std::getline(file, fileLine))
		{
			lineProcessor(fileLine);
		}
		file.close();
	}
	void forLineIn(const std::filesystem::path fileName, std::function<void(std::string_view)> lineProcessor)
	{
		bool noStop = false;
		forLineIn(std::move(fileName), std::move(lineProcessor), noStop);
	}

	void forSplitLineIn(const std::filesystem::path fileName, std::string_view delimiter, std::function<void(std::vector<std::string_view>)> vectorProcessor)
	{
		forLineIn(fileName,
			[&](auto line) {
				vectorProcessor(String::split_view(line, delimiter));
			});
	}

} // namespace JSL::IO
