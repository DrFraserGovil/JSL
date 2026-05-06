
#pragma once
#include <string>
#include <vector>
#include <type_traits> // For std::is_arithmetic_v, std::enable_if_t, etc.
#include <sstream>     // For std::stringstream for more controlled numeric to_string
#include <string_view> // For delimiter in vector case
 #include <concepts>
#include <ranges>
namespace JSL::String
{
    /*! @brief Internal interface for Represent. 
        
    @details As with ParseTo we have to provide a wrapper struct to allow vector partial specialisation. 
        Default struct only applies to numeric types. Overloads handle the others
    */


    template<typename T>
    struct RepresentStruct
    {
    };


    //macro to provide specialisations without boilerplate muddling it all up
    #define JSL_HAS_SPECIALISATION(type) \
        template<> struct RepresentStruct<type>{static std::string stringify (const type & value);}; 
    
    #define JSL_HAS_SPECIALISATION_NOCONST(type) \
        template<> struct RepresentStruct<type>{static std::string stringify ( type & value);};


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


    namespace internal
    {
        template<typename T>
        concept StringifiableRange = std::ranges::range<T> && 
                    !std::convertible_to<T, std::string_view>;
    }

    template<internal::StringifiableRange T>
    struct RepresentStruct<T>
    {
        //! @brief Specialization for `std::vector<T_Inner>`
        //! @details Calls with default delimiter
        static std::string stringify(const T& container)
        {
            return stringify(container,", ");
        }
        
        //! @brief Specialization for `std::vector<T_Inner>`
        //! @details Calls with custom delimiter
        static std::string stringify(const T& container, std::string_view delimiter)
        {
            std::ostringstream result;
            result << "["; // A default endcap

            bool first = true;
            for (const auto& item : container)
            {
                if (!first)
                {
                    result << delimiter;
                }
                
                // Extract the inner type and recurse
                using InnerType = std::remove_cvref_t<decltype(item)>;
                result << RepresentStruct<InnerType>::stringify(item);
                
                first = false;
            }

            result << "]";
            return result.str();
        }
    };

    /*! @brief Converts the object into a string. 
        @brief If the object is a vector, the default delimiter - "," - is used.
        @param obj An object (usually a numeric, boolean or vector) to be converted
        @returns A string representation
    */
    template<typename T>
    std::string inline represent(T obj)
    {
        return RepresentStruct<T>::stringify(obj);
    };

    /*! @brief Converts a vector into a string
        @param obj An object (usually a numeric, boolean or vector) to be converted
        @param delimiter The character(s) which delimit the elements of the vector
        @returns A string representation
    */
    template<typename T>
    std::string inline represent(T obj,std::string delimiter)
    {
        return RepresentStruct<T>::stringify(obj,delimiter);
    };
}