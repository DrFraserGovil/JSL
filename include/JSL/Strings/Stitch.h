#pragma once
#include <string>
#include <vector>
#include <JSL/Strings/Represent.h>
#include <sstream>
#include <JSL/internal/concepts.h>
namespace JSL::String
{
    //we use this for containers which can be accessed using []
    template<JSL::internal::IndexableRange container>
    std::string stitch(const container & vec, size_t begin, size_t end, std::string_view delim)
    {
        if (begin >= end || begin >= vec.size())
        {
            return "";
        }
        std::ostringstream os;
        
        os << represent(vec[begin]);
        for (size_t i = begin + 1; i < end && i < vec.size(); ++i)
        {
            os << delim  << represent(vec[i]);
        }
        return os.str();
    }

    //this is a more generic one which just dumps everything in
    template<JSL::internal::SearchableRange container>
    std::string stitch(const container & vec, std::string_view delim)
    {
        std::ostringstream os;
        
        bool first = true;
        for (auto & v : vec)
        {
            if (!first)
            {
                os << delim;
            }
            else
            {
                first = false;
            }
            os << represent(v);
        }
        return os.str();
    }
     
}