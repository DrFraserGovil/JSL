#include <JSL/Strings/ParseTo.h>
#include "../utils/jsl_error.h"
namespace JSL
{
    namespace detail
    {
        
        void CheckErrors(std::from_chars_result & result,std::string_view sv,std::string_view typeName)
        {
            if (result.ec == std::errc() &&  (result.ptr != sv.data() + sv.size()))
            { 
                internal::FatalError("Could not complete conversion") << "Partial conversion of `" << sv << "` to type " << typeName << " unconverted characters were: " << std::string_view(result.ptr, sv.data() + sv.size() - result.ptr);
            }
            else if (result.ec == std::errc::invalid_argument) 
            {
                internal::FatalError("Could not complete conversion") <<  "Error: Invalid argument for conversion: '" << sv   << "` to type " << typeName<< "\n";
                
            } 
            else if (result.ec == std::errc::result_out_of_range)
            {
                internal::FatalError("Could not complete conversion") <<  "Error: Result out of range for conversion: '" << sv << "` to type " << typeName << "\n";
            }
        }

        void RejectEmpty(std::string_view sv,std::string_view typeName)
        {
            if (sv.empty()) 
            {
                 internal::FatalError("Could not complete conversion") << "Cannot convert an empty string to to type " <<typeName;
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
        


        #define PROVIDE_SPECIALISATION(type,...) \
            type Converter<type>::internalConvert(std::string_view sv){__VA_ARGS__}

 

        PROVIDE_SPECIALISATION(std::string,
             //we assume that conversion to strings does not preserve leading or trailing whitespace, as this is spurious for strings
             return JSL::trim(sv);
        )

        PROVIDE_SPECIALISATION(bool,
            auto snap = trim_view(sv,"//");
            if (snap == "1" || insensitiveEquals(snap,"true"))
            {
                return true;
            }
            if (snap == "0" || insensitiveEquals(snap,"false"))
            {
                return false;
            }

           internal::FatalError("Cannot complete string-boolean conversion") << "Cannot convert string " << sv << "to boolean";
           return false;
        );

        PROVIDE_SPECIALISATION(char,
            // Trim whitespace first
            sv = trim_view(sv,"//");
            // A single char conversion should only accept a single character string_view
            if (sv.length() != 1) {
                internal::FatalError("Cannot complete string-char conversion")  << "Cannot convert string_view '" << sv << "' to char: Expected a single character.";
            }
            return sv[0];
        )

        #if defined(__clang__) && defined(__APPLE__)
            PROVIDE_SPECIALISATION(double,
            
                sv = trim_view(sv,"//");
                if (sv.empty()) 
                {
                    internal::FatalError("Cannot complete string-double conversion") << "Cannot convert an empty string to type double" 
                } 

                try
                {
                    std::string s_temp(sv);
                    size_t pos = 0;
                    double output = std::stod(s_temp, &pos);

                    if (pos != s_temp.length())
                    {
                        internal::FatalError("Trailing characters in double parsing")  << "Partial conversion of `" << sv << "` to double; unconverted characters were: `" << s_temp.substr(pos) << "`";
                    }
                    return output;
                }
                catch (const std::out_of_range& e)
                {
                    internal::FatalError("Out-of-range error in double conversion") << "Error: Result out of range for conversion: '" << sv << "` to double\n";
                    
                }
                catch (const std::invalid_argument& e)
                {
                    internal::FatalError("Could not complete conversion (invalid format).") << "Error: Invalid argument for conversion: '" << sv << "` to double\n";
                }
            
            )
        #endif

    }

}

