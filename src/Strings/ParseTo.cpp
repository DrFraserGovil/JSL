#include <JSL/Strings/ParseTo.h>
#include <JSL/internal/error.h>
#include <JSL/Strings/Cases.h>
namespace JSL::String
{
	using namespace JSL::internal; //for errors
	namespace internal
	{
		
		void CheckErrors(std::from_chars_result & result,std::string_view sv,std::string_view typeName)
		{
			if (result.ec == std::errc() &&  (result.ptr != sv.data() + sv.size()))
			{ 
				FatalError("Could not complete conversion", JSL_LOCATION) << "Partial conversion of `" << sv << "` to type " << typeName << " unconverted characters were: " << std::string_view(result.ptr, sv.data() + sv.size() - result.ptr);
			}
			else if (result.ec == std::errc::invalid_argument) 
			{
				FatalError("Could not complete conversion", JSL_LOCATION) <<  "Error: Invalid argument for conversion: '" << sv   << "` to type " << typeName<< "\n";
				
			} 
			else if (result.ec == std::errc::result_out_of_range)
			{
				FatalError("Could not complete conversion", JSL_LOCATION) <<  "Error: Result out of range for conversion: '" << sv << "` to type " << typeName << "\n";
			}
		}

		void RejectEmpty(std::string_view sv,std::string_view typeName)
		{
			if (sv.empty()) 
			{
				 FatalError("Could not complete conversion", JSL_LOCATION) << "Cannot convert an empty string to to type " <<typeName;
			} 
			if (sv == "__bool_tag__" && typeName != typeid(bool).name())
			{
				FatalError("Could not complete conversion", JSL_LOCATION) << "The string `__bool_tag__` is reserved for boolean conversion, and cannot be converted to type " << typeName;
			}
		}   


		std::string_view StripEndCaps(std::string_view sv)
		{
			size_t start = 0;
			size_t end = sv.size();
			// Check for common opening brackets
			if (sv[0] == '[' || sv[0] == '{' || sv[0] == '(')
			{
				start = 1;
			}
			// Check for common closing brackets
			if ( (sv[end - 1] == ']' || sv[end - 1] == '}' || sv[end - 1] == ')'))
			{
				end -= 1;
			}

			
			return sv.substr(start,end-start);
		}

		std::vector<std::string_view> tokenize(std::string_view sv,std::string_view delimiter, std::string_view typeName)
		{
			sv=trim_view(sv);
			RejectEmpty(sv,typeName);
			//! We allow vectors to be wrapped in either [], {} or (). This function removes them for internal use.
			sv = StripEndCaps(sv);

			auto tokens = split_view(sv,delimiter);
			std::vector<std::string_view> out;
			for (auto & t : tokens)
			{
				auto r = trim_view(t);
				if (!r.empty())
				{
					out.push_back(r);
				}
			}
			return out;
		}
	}



	template<>
	char ParseTo(std::string_view sv)
	{
		sv=trim_view(sv);
		internal::RejectEmpty(sv,typeid(char).name());
		if (sv.length() != 1) 
		{
			JSL::internal::FatalError("Cannot complete string-char conversion",JSL_LOCATION)  << "Cannot convert string_view '" << sv << "' to char: Expected a single character.";
		}
		return sv[0];
	}

	template<>
	bool ParseTo(std::string_view sv)
	{
		sv=trim_view(sv);
		internal::RejectEmpty(sv, typeid(bool).name());

		if (sv == "1" || iEquals(sv,"true") || iEquals(sv,"yes") || iEquals(sv,"on") || iEquals(sv,"__bool_tag__"))
		{
			return true;
		}
		if (sv == "0" || iEquals(sv,"false")|| iEquals(sv,"no") || iEquals(sv,"off"))
		{
			return false;
		}
		JSL::internal::FatalError("Cannot complete string-boolean conversion", JSL_LOCATION) << "Cannot convert string " << sv << " to boolean";
		return false;//dead code, but suppresses compiler warning
	}



	#if defined(__clang__) && defined(__APPLE__)
		template<>
		double inline ParseTo(std::string_view sv)
		{
			sv=trim_view(sv);
			internal::RejectEmpty(sv, typeid(double).name());
			 try
                {
                    std::string s_temp(sv);
                    size_t pos = 0;
                    double output = std::stod(s_temp, &pos);

                    if (pos != s_temp.length())
                    {
                        FatalError("Trailing characters in double parsing",JSL_LOCATION)  << "Partial conversion of `" << sv << "` to double; unconverted characters were: `" << s_temp.substr(pos) << "`";
                    }
                    return output;
                }
                catch (const std::out_of_range& e)
                {
                  	JSL::internal::FatalError("Out-of-range error in double conversion",JSL_LOCATION) << "Error: Result out of range for conversion: '" << sv << "` to double\n";
                    
                }
                catch (const std::invalid_argument& e)
                {
                    JSL::internal::FatalError("Could not complete conversion (invalid format).",JSL_LOCATION) << "Error: Invalid argument for conversion: '" << sv << "` to double\n";
                }
		}
	#endif


	#undef BOILERPLATE_PARSE
}

