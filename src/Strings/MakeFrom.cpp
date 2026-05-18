#include <JSL/Strings/MakeFrom.h>


namespace JSL::String
{
    //Provide specialisations and definitions to the Represent objects

    #define PROVIDE_SPECIALISATION(type,...) \
        std::string RepresentStruct<type>::stringify(const type & value){__VA_ARGS__} \

    #define PROVIDE_SPECIALISATION_NOCONST(type,...) \
        std::string RepresentStruct<type>::stringify(type & value){__VA_ARGS__} \
 
        
        
    //all numeric types get the same thing - just need one for each
    #define NUMERIC_SPECIALISATION(type)\
        PROVIDE_SPECIALISATION(type,return std::to_string(value);)
    
    NUMERIC_SPECIALISATION(int);
    NUMERIC_SPECIALISATION(unsigned int);
    NUMERIC_SPECIALISATION(long);
    NUMERIC_SPECIALISATION(unsigned long);
    NUMERIC_SPECIALISATION(long long);
    NUMERIC_SPECIALISATION(unsigned long long);
    NUMERIC_SPECIALISATION(float);
    NUMERIC_SPECIALISATION(double);
    NUMERIC_SPECIALISATION(long double);


    //here is the actual specialisation

    PROVIDE_SPECIALISATION(bool,
        return value ? "true" : "false";
    )

    PROVIDE_SPECIALISATION(char,
        return std::string(1, value); 
    );
    PROVIDE_SPECIALISATION(char*,
        return value ? std::string(value) : std::string("");
    );
    PROVIDE_SPECIALISATION_NOCONST(const char*,
        return value ? std::string(value) : std::string("");
    );
    PROVIDE_SPECIALISATION(std::string,
        return value;
    );
    PROVIDE_SPECIALISATION(std::string_view,
        return (std::string)value;
    );

    #undef PROVIDE_SPECIALISATION
    #undef NUMERIC_SPECIALISATIOn
}