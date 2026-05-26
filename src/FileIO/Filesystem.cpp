#include <JSL/FileIO/Filesystem.h>

namespace JSL::IO
{
	namespace fs = std::filesystem;

	Policy DefaultPolicy = Strict;




	report mkdir(const std::filesystem::path directory,Policy policy)
	{
		if (fs::exists(directory))
		{
			if (policy == Strict)
			{
				return {false,"Cannot create " + directory.string() + ": already exists"};
			}
			else
			{
				return {true,""}; //a quiet policy treats an extant directory as a full success
			}
		}

		std::error_code ec;
		std::filesystem::create_directories(directory,ec);
		if (ec)
		{
			return {false, "ERROR: " + ec.message()};
		}

		return {true,""};

	}

	report internalErase(const std::filesystem::path path,Policy policy)
	{
		if (!fs::exists(path))
		{
			if (policy == Strict)
			{
				return {false, "Cannot remove " + path.string() + ": does not exist"};
			}
			else
			{
				return {true,""};
			}
		}
		std::error_code ec;
		std::filesystem::remove_all(path,ec);
		if (ec)
		{
			return {false, "ERROR: " + ec.message()};
		}

		return {true,""};
	}

	report remove(const std::filesystem::path pathToFile,Policy policy)
	{
		if (fs::is_directory(pathToFile))
		{
			return {false,"Deleting directory " + pathToFile.string() + " always requires an explicit call to removeDirectory"};
		}
		return internalErase(pathToFile,policy); //removeDirectory is just an aggressive delete, so we can duplicate here
	}

	report removeDirectory(const std::filesystem::path pathToDirectory,Policy policy)
	{
		if(!fs::is_directory(pathToDirectory) && policy == Strict)
		{
			return {false,pathToDirectory.string() + " is not a directory; the Strict policy forbids removeDirectory from removing it."};
		}
		return internalErase(pathToDirectory,policy);
	}
}