#include <JSL/internal/error.h>

#include <algorithm>
#include <filesystem>
#include <stdexcept>

namespace JSL::internal
{
	namespace fs = std::filesystem;
	std::string truncatedFilePath(const std::string &fullPath, std::string_view target)
	{
		std::filesystem::path p(fullPath);

		auto it = std::find(p.begin(), p.end(), target);

		if (it != p.end())
		{
			fs::path result;
			for (; it != p.end(); ++it)
			{
				result = result / *it;
			}
			return result.string();
		}

		return fullPath; // Or return empty if not found
	}

	FatalError::FatalError(std::string msg, int callingLine, const std::string &callingFunction, std::string callingFile) : Summary(msg)
	{
		auto shortFile = truncatedFilePath(callingFile, "JSL");
		line = callingLine;
		function = callingFunction;
		file = shortFile;
		Buffer << file << "/" << function << "/" << line << ": ";
	}

	FatalError::~FatalError() noexcept(false)
	{
		throw std::runtime_error(Summary + "\t" + Buffer.str());
		return;
	}

} // namespace JSL::internal
