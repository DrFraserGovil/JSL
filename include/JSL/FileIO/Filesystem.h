#pragma once
#include <filesystem>
namespace JSL::IO
{


	struct report
	{
		bool Successful;
		std::string Message;
	};

	enum Policy
	{
		Strict,
		Quiet
	};

	extern Policy DefaultPolicy;

	report mkdir(const std::filesystem::path directory, Policy policy = DefaultPolicy);

	report remove(const std::filesystem::path pathToFile, Policy polict = DefaultPolicy);

	/*
		An explicit function that forces a user to acknowledge they're deleting a whole directory and all its contents
	*/
	report removeDirectory(const std::filesystem::path pathToDirectory, Policy policy = DefaultPolicy);

}