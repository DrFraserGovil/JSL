
#pragma once
#include <string>
#include <vector>
#include <type_traits> // For std::is_arithmetic_v, std::enable_if_t, etc.
#include <sstream>     // For std::stringstream for more controlled numeric to_string
#include <string_view> // For delimiter in vector case

namespace JSL
{
    /*! @brief Internal interface for MakeString. 
        
    @details As with ParseTo we have to provide a wrapper struct to allow vector partial specialisation. 
        Default struct only applies to numeric types. Overloads handle the others
    */


    template<typename T>
    struct MakeStringStruct
    {
    };


    //macro to provide specialisations without boilerplate muddling it all up
    #define JSL_HAS_SPECIALISATION(type) \
        template<> struct MakeStringStruct<type>{static std::string stringify (const type & value);}; 
    
    #define JSL_HAS_SPECIALISATION_NOCONST(type) \
        template<> struct MakeStringStruct<type>{static std::string stringify ( type & value);};


    JSL_HAS_SPECIALISATION(bool);


    //a whole bunch of specialisations
    JSL_HAS_SPECIALISATION(int);
    JSL_HAS_SPECIALISATION(float);
    JSL_HAS_SPECIALISATION(double);
    JSL_HAS_SPECIALISATION(long);
    JSL_HAS_SPECIALISATION(long long);
    JSL_HAS_SPECIALISATION(long double);
    JSL_HAS_SPECIALISATION(unsigned long long);
    JSL_HAS_SPECIALISATION(unsigned long);
    JSL_HAS_SPECIALISATION(unsigned int);

    JSL_HAS_SPECIALISATION(char);
    JSL_HAS_SPECIALISATION(char*);
    JSL_HAS_SPECIALISATION_NOCONST(const char*);

    JSL_HAS_SPECIALISATION(std::string);
    JSL_HAS_SPECIALISATION(std::string_view);

    #undef JSL_HAS_SPECIALISATION


    template<typename T_Inner>
    struct MakeStringStruct<std::vector<T_Inner>>
    {
        //! @brief Specialization for `std::vector<T_Inner>`
        //! @details Calls with default delimiter
        static std::string stringify(const std::vector<T_Inner>& vec)
        {
            return stringify(vec,", ");
        }
        
        //! @brief Specialization for `std::vector<T_Inner>`
        //! @details Calls with custom delimiter
        static std::string stringify(const std::vector<T_Inner>& vec, std::string_view delimiter_str)
        {
            std::string result = "["; // A default endcap

            for (size_t i = 0; i < vec.size(); ++i)
            {
                result += MakeStringStruct<T_Inner>::stringify(vec[i]);
                if (i < vec.size() - 1)
                {
                    result += delimiter_str;
                }
            }
            result += "]";
            return result;
        }
    };

    /*! @brief Converts the object into a string. 
        @brief If the object is a vector, the default delimiter - "," - is used.
        @param obj An object (usually a numeric, boolean or vector) to be converted
        @returns A string representation
    */
    template<typename T>
    std::string inline MakeString(T obj)
    {
        return MakeStringStruct<T>::stringify(obj);
    };

    /*! @brief Converts a vector into a string
        @param obj An object (usually a numeric, boolean or vector) to be converted
        @param delimiter The character(s) which delimit the elements of the vector
        @returns A string representation
    */
    template<typename T>
    std::string inline MakeString(T obj,std::string delimiter)
    {
        return MakeStringStruct<T>::stringify(obj,delimiter);
    };
}