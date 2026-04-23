#include <JSL/Strings/Cases.h>
#include <algorithm>
namespace JSL
{
	void toUpper(std::string & input)
	{
		for (auto & c : input)
		{
			c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
		}
	}
	void toLower(std::string & input)
	{
		for (auto & c : input)
		{
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}
	}

	std::string getUpper(std::string_view input)
	{
		std::string copy{input};
		toUpper(copy);
		return copy;
	}
	std::string getLower(std::string_view input)
	{
		std::string copy{input};
		toLower(copy);
		return copy;
	}

	bool iEquals(std::string_view a, std::string_view b)
    {
        return a.size() == b.size() &&
                std::equal(a.begin(), a.end(), b.begin(), [](char a, char b){
                return std::tolower(static_cast<unsigned char>(a)) ==
                    std::tolower(static_cast<unsigned char>(b));
            });
    }
}