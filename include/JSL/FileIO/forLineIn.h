#pragma once
#include <functional>
#include <filesystem>


namespace JSL
{
    void forLineIn(const std::filesystem::path fileName, std::function<void(std::string_view)> lineProcessor);
    

    void forSplitLineIn(const std::string & fileName, std::string_view delimiter,  std::function<void(std::vector<std::string_view>)> vectorProcessor);




    template <typename... Ts, typename Func>
    void forLineTupleIn(const std::string & fileName, std::string_view delimiter, Func tupleProcessor)
    {
        forLineIn(fileName,
            [&, delimiter](auto  line)
            {
                auto sv_vec = split_view(line, delimiter);
                // Convert the vector of string_views into the desired tuple
                std::tuple<Ts...> parsed_tuple = ParseTo<Ts...>(sv_vec);
                tupleProcessor(parsed_tuple);
            }
        );
    }
}