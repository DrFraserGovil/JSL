#include <JSL/FileIO/forLineIn.h>
#include <fstream>
#include <string_view>
#include <JSL/Strings.h>
#include "../utils/jsl_error.h"
namespace JSL
{
    void forLineIn(const std::filesystem::path fileName, std::function<void(std::string_view)> lineProcessor)
    {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            internal::FatalError("Could not open file") << "Could not find the file '" << fileName << "'.\nPlease provide a valid filepath.";
        }

        std::string fileLine;
        while (std::getline(file, fileLine))
        {
            lineProcessor(fileLine);
        }
        file.close();
    }

    void forSplitLineIn(const std::string & fileName, std::string_view delimiter,  std::function<void(std::vector<std::string_view>)> vectorProcessor) 
    {
        forLineIn(fileName,
            [&](auto line)
            {
                vectorProcessor(split_view(line,delimiter));
            }
        );
    }




    template <typename... Ts, typename Func>
    void forLineTupleIn(const std::string & fileName, std::string_view delimiter, Func tupleProcessor)
    {
        forLineIn(fileName,
            // Important: `line` is taken by value here (`std::string line`).
            // This ensures the `std::string` object (which split's string_views refer to)
            // lives for the entire duration that `sv_vec` and the converted `parsed_tuple` are used,
            // including inside the `tupleProcessor` lambda.
            [&, delimiter](std::string & line)
            {
                std::vector<std::string_view> sv_vec = split(line, delimiter);
                // Convert the vector of string_views into the desired tuple
                std::tuple<Ts...> parsed_tuple = ParseTo<Ts...>(sv_vec);
                // Pass the fully typed tuple to the user's lambda
                tupleProcessor(parsed_tuple);
            }
        );
    }
}