#include "JSL/Strings/MakeString.h"


namespace JSL
{
    #define PROVIDE_SPECIALISATION(type,...) \
        std::string MakeStringStruct<type>::stringify(const type & value){__VA_ARGS__} \
 
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




    PROVIDE_SPECIALISATION(bool,
        return value ? "true" : "false";
    )

    PROVIDE_SPECIALISATION(char,
        return std::string(1, value); 
    );
    PROVIDE_SPECIALISATION(char*,
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