#pragma once
#include <sstream>
#define JSL_LOCATION __LINE__, __func__, __FILE__
namespace JSL::internal
{
	class LibraryError
	{
	  private:
		const std::string Summary;
		std::ostringstream Buffer;
		int line;
		std::string function;
		std::string file;

	  public:
		LibraryError(std::string msg, int callingLine, const std::string &callingFunction, std::string callingFile);

		template <class T>
		LibraryError &operator<<(const T &input)
		{
			Buffer << input;
			return *this;
		}
		~LibraryError() noexcept(false);
	};
} // namespace JSL::internal
