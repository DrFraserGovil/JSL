#pragma once
#include <iostream>
#include <string_view>
#include <vector>
#include <charconv>
#include "Manipulate.h"
#include "Cases.h"
#include <JSL/internal/error.h>

/*
    This file provides a robust return-value interface for converting string-views (and, implicitly, strings) into a candidate type
    This type can be any value accepted by std::from_chars, or those which can be implicitly converted to from that type

    Converter also handles conversion of multiple values at once (of the same type) into a vector.

    Special overloads are provided that allow direct stringview->string conversion.
*/

namespace JSL
{
    namespace detail
    {
        //helper functions for the template converter class
        void CheckErrors(std::from_chars_result & result,std::string_view sv,std::string_view typeName);
        void RejectEmpty(std::string_view sv,std::string_view typeName);
        std::string_view StripEndCaps(std::string_view sv);

        /*!
            The backend structure for the convert function. 
            @details Required because functions are not able to undergo partial specialisation, whilst classes can. This therefore amounts to some compile-time wizardry.
        */
        template<typename T>
        struct Converter
        {
            /*!
                The default conversion operator -- where the actual conversion happens. 
                @details Specialisations (i.e. for new convert-types) must define their own version of this function
                @param sv A string_view (implicitly converted from std::strings) representing a value to be converted
                @returns A value of type T corresponding to the input string
                @throws logic_error If the characters could not be converted into numerics (i.e. ab)
                @throws logic_error If some, but not all, characters could be converted into numerics (i.e. 123ab)
                @throws logic_error If the result is out of range (i.e. convert<int>(INT_MAX + 10))
            */
            static T internalConvert(std::string_view sv)
            {
                sv = trim_view(sv,"//");
                RejectEmpty(sv,typeid(T).name());
                
                //create an object and read from_chars into it. Some implicit type conversion is allowed here (i.e. if T is a bool)
                T output{};
                auto result = std::from_chars(sv.data(), sv.data() + sv.size(),output);

                CheckErrors(result,sv,typeid(T).name());		
                        
                return output;
            }

            
        };
    

        //macro to provide specialisations without boilerplate muddling it all up
        #define JSL_HAS_SPECIALISATION(type) \
            template<> struct Converter<type>\
            {\
                static type internalConvert(std::string_view sv); \
            };


        JSL_HAS_SPECIALISATION(std::string);
        JSL_HAS_SPECIALISATION(bool);
        JSL_HAS_SPECIALISATION(char);
        
        
        /*! For reasons unknown to me, Apple-Clang does not fully implement the C++ standard. std::from_chars does not work on non-integral types.
            This is really, really annoying that this is necessary. 
            Must therefore resort to using the (slower, worse) std::stod for Apple people 
        */
        #if defined(__clang__) && defined(__APPLE__)
            JSL_HAS_SPECIALISATION(double);
        #endif

        #undef JSL_HAS_SPECIALISATIOn

        /*! A partial specialisation of Converter<T> to allow the extraction of vectors.
            @details Functionally, this acts as a wrapper, iteratively calling Converter<T_Inner> on the values extracted
        */
        template <typename T_Inner>
        struct Converter<std::vector<T_Inner>> 
        {
            //! Calls internalConvert(std::string_view, std::string_view,typename) with the default delimiter (a comma)
            static std::vector<T_Inner> internalConvert(std::string_view sv,typename std::enable_if_t<!std::is_same_v<T_Inner, char>>* = nullptr)
            {
                return internalConvert(sv, ",", nullptr); // Pass nullptr for the dummy SFINAE arg
            }

            /*!
                @brief The iterative converter for vector types -- loops over the split-vector and converts the internal types.
                @details The `typename` argument allows the function to disable itself at compile time if T_Inner is a char, thus inducing a Substitution Failure. Through SFINAE principles, this allows std::string (internally a std::vector<char> in many ways) to be ignored by this, and thus picked up by the standard converter
            */
            static std::vector<T_Inner> internalConvert(std::string_view sv, std::string_view element_delimiter,typename std::enable_if_t<!std::is_same_v<T_Inner, char>>* = nullptr) 
            {
                sv = trim_view(sv,"//");
                if (sv.empty()) 
                {
                    internal::FatalError("Cannot parse empty string") << "Empty-vectors can only be instantiated if they have enclosing braces -- empty strings are not valid.";
                }


                 //! We allow vectors to be wrapped in either [], {} or (). This function removes them for internal use.
                //! @warning We do *not* do any form of parsing or checking to allow nested braces. End caps are purely for user readability.
                sv = StripEndCaps(sv);

                if (sv.empty()) 
                {
                    return std::vector<T_Inner>{};
                } 
                std::vector<T_Inner> result_vec;
                std::vector<std::string_view> elements_sv = split_view(sv, element_delimiter);
                int i = 0;
                for (const auto& elem_sv : elements_sv)
                {
                    if (elem_sv.empty())
                    {
                        internal::FatalError("Cannot parse empty string")<< "Element " << i << " of the vector " << sv << " is empty.\nVector-conversion does not accept empty strings (even if empty strings are allowed for base type";
                    }
                    ++i;
                    result_vec.push_back(Converter<T_Inner>::internalConvert(elem_sv));
                }
                return result_vec;
            }          
        };


       
    }


    /*! @brief Converts input strings into values of different types
        @details Provides a larger wrapper for the std::from_chars, and extends the functionality to more data types
        @param sv A string_view (implicitly converted from std::strings) representing a value to be converted
        @returns A value of type T corresponding to the input string
    */
    template <typename T>
    T inline ParseTo(std::string_view sv)
    {
        return detail::Converter<T>::internalConvert(sv);
    }



    //Define a nice C++20 concept which defines a 'vector type' as any std::vector<T>, but excludes std::string from being too eager
        template<typename T>
        struct is_vector : std::false_type {};
        template<typename T, typename A>
        struct is_vector<std::vector<T, A>> : std::true_type {};
    template<typename T>
    concept VectorType = is_vector<T>::value;
    //end concept  

    template<VectorType T>
    T inline ParseTo(std::string_view sv, std::string_view delimiter)
    {   
        return detail::Converter<T>::internalConvert(sv, delimiter);
    }



    //have to reopen namespace because this needs ParseTo to be visible
    namespace internal
    {
        // Helper function to create a tuple from a vector of string_views
        // This uses std::index_sequence and a fold expression for compile-time unpacking
        template <typename... Ts, std::size_t... Is>
        std::tuple<Ts...> ImplicitTupleConverter(const std::vector<std::string_view>& sv_vec,std::index_sequence<Is...>)
        {
            return std::make_tuple(ParseTo<Ts>(sv_vec[Is])...);
        }
    }

    template <typename... Ts>
    std::tuple<Ts...> inline ParseTo(const std::vector<std::string_view>& sv_vec)
    {
        constexpr std::size_t expected_size = sizeof...(Ts);
        if (sv_vec.size() != expected_size) 
        {
            internal::FatalError("Tuple conversion: Incorrect token count.") << "Tuple conversion error: Token count in vector (" << sv_vec.size()<< ") does not equal tuple size (" << expected_size <<")";
        }
        

        // Now, call the implementation which will do the actual conversions
        return internal::ImplicitTupleConverter<Ts...>(sv_vec, std::index_sequence_for<Ts...>{});
    }
}

