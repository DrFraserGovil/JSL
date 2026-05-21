#include <JSL/Strings/ParseTo.h>
#include <JSL/internal/error.h>
#include <JSL/Strings/Cases.h>
#include <JSL/Concepts.h> //needed for the nullstring
#include <JSL/Strings/Manipulate.h>
namespace JSL::String
{
	using namespace JSL::internal; //for errors
	namespace internal
	{
		const std::vector<std::pair<char,char>> endCaps{std::pair<char,char>{'[',']'},std::pair<char,char>{'(',')'},std::pair<char,char>{'{','}'}}; 
		void checkErrors(std::from_chars_result & result,std::string_view sv,std::string_view typeName)
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

		void RejectEmpty(std::string_view sv,std::string_view typeName, bool isOptional)
		{
			if (sv.empty()) 
			{
				 FatalError("Could not complete conversion", JSL_LOCATION) << "Cannot convert an empty string to to type " <<typeName;
			} 
			if (sv == "__bool_tag__" && typeName != typeid(bool).name())
			{
				FatalError("Could not complete conversion", JSL_LOCATION) << "The string `__bool_tag__` is reserved for boolean conversion, and cannot be converted to type " << typeName;
			}
			if (sv == JSL_NULL_STRING && !isOptional)
			{
				FatalError("Could not complete conversion", JSL_LOCATION) << "String string `__none__` is reserved for std::optional types, and cannot be converted to type " << typeName;
			}
		}   

		void processInput(std::string_view & sv, std::string_view type, bool rejectEmpty, bool isOptional)
		{
			sv = String::trim_view(sv);
			if (rejectEmpty)
			{
				RejectEmpty(sv,type, isOptional);
			}
		}



		size_t jumpToPartner(std::string_view sv, size_t idx,char ender)
		{
			char opener = sv[idx];
			
			int level = 1;	
			for (size_t pos = idx + 1; pos < sv.size(); ++pos)
			{
				if (sv[pos] == opener)
				{
					++level;
				}
				if (sv[pos] == ender)
				{
					--level;
					if (level == 0)
					{
						return pos;
					}
				}
			}

			JSL::internal::FatalError("Mismatched braces",JSL_LOCATION) << "Could not find " << ender << " character associated with the opener at position " << idx <<" in " << sv;
			return idx;
		}

		std::string_view StripEndCaps(std::string_view sv)
		{
			size_t end = sv.size();

			for (auto & pair : endCaps)
			{
				if (sv[0] == pair.first)
				{
					auto idx = jumpToPartner(sv,0,pair.second);
					if (idx == end-1) // i.e. if we've matched with the end character, then strip
					{
						return sv.substr(1,end-2);
					}
					else // this is the case where nestings get in the way
					{
						return sv;
					}
				}
			}	
			return sv; // this is no outer braces
		}

		std::vector<std::string_view> recursetokens(std::string_view sv)
		{
			processInput(sv,"vector",true);	
			sv= StripEndCaps(sv);
			std::vector<std::string_view> out;
			
			size_t grab = 0;
			while (grab < sv.size())
			{
				for (auto & cap : endCaps)
				{
					if (sv[grab] == cap.first)
					{
						auto end = jumpToPartner(sv,grab,cap.second);
						out.emplace_back(sv.substr(grab,end+1-grab));
						grab = end;
						break;
					}
				}
				++grab;
			}
	

			return out;
		}


		std::vector<std::string_view> tokenize(std::string_view sv,std::string_view delimiter, std::string_view typeName)
		{
			processInput(sv,typeName,true);
			sv = StripEndCaps(sv);
			std::vector<std::string_view> out;
			auto tokens = split_view(sv,delimiter);
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
		internal::processInput(sv,"char",true);
		if (sv.length() != 1) 
		{
			JSL::internal::FatalError("Cannot complete string-char conversion",JSL_LOCATION)  << "Cannot convert string_view '" << sv << "' to char: Expected a single character.";
		}
		return sv[0];
	}

	template<>
	bool ParseTo(std::string_view sv)
	{
		internal::processInput(sv,"bool",true);
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
			internal::processInput(sv,"double",true);
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


}

