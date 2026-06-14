#pragma once
#include <functional>
#include <filesystem>
#include <vector>
#include <JSL/Strings/Manipulate.h>
#include <JSL/Strings/ParseTo.h>

namespace JSL::IO
{

    /*!
     * @brief Opens a textfile in read mode and iterates line-by-line, calling the callback function on each line
     * @param fileName The file to be opened
     * @param lineProcessor A function (i.e. a lambda) which operates on a temporary view of the line
     * @warning For performance reasons, the callback function acts on a view of the line, which passes out of scope as soon as the line callback ends. To preserve a copy of a line, it must be directly constructed into an std::string object
     * @throws runtime_error if the file cannot be located or opened. 
     */
    void forLineIn(const std::filesystem::path fileName, std::function<void(std::string_view)> lineProcessor);
    
    /*!
     * @brief Opens a textfile in read mode and iterates line-by-line, splitting the line into a vector based on a delimiter, and passing this to a callback function
     * @param fileName The file to be opened
     * @param delimiter The character sequence used to indicate the end of a chunk (not included in the line view)
     * @param vectorProcessor A function (i.e. a lambda) which operates on a series of views into the line
     * @warning For performance reasons, the callback function acts on a vector-of-views of the line, which passes out of scope as soon as the line callback ends. To preserve a copy of a word, it must be directly constructed into an std::string object
     * @throws runtime_error if the file cannot be located or opened. 
     */
    void forSplitLineIn(const std::filesystem::path fileName, std::string_view delimiter,  std::function<void(std::vector<std::string_view>)> vectorProcessor);

 

    /*!
     * @brief Reads a file into memory, and then parses them into a specified type, before applying a callback to the converted object
     * @tparam T A type supported by the JSL::String::ParseTo interface
     * @param fileName The file to be read
     * @param callback A function which is applied to each line (after converting it to type T)
     */
    template<typename T>
    void forConvertedLineIn(const std::filesystem::path fileName, std::function<void(T)> callback)
    {
        forLineIn(fileName,
        {
            [callback](auto line)
            {
                callback(JSL::String::ParseTo<T>(line));
            }
        });
    }
    
    /*!
     * @brief Reads a file into memory, and then parses them into a specified type using the provided delimiter sequence, before applying a callback to the converted object
     * @tparam T A type supported by the JSL::String::ParseTo interface with a delimiter sequence (usually STL containers like std::vector)
     * @param fileName The file to be read
     * @param delimiter The character sequence used to separate elements during the conversion to type T
     * @param callback A function which is applied to each line (after converting it to type T)
     */
    template<class T>
    void forConvertedLineIn(const std::filesystem::path fileName, std::string_view delimiter, std::function<void(T)> callback)
    {
        forLineIn(fileName,[&](auto line)
        {
            T tmp = JSL::String::ParseTo<T>(line,delimiter);
            callback(tmp);
        });
    }
    /*!
     * @brief Opens a textfile for reading, converts each line into a sequence of types specified by the template, and then activates a callback function with these typed arguments
     * @tparam ...Ts A list of types supported by the JSL::String::ParseTo function 
     * @tparam Func  A callable object which accepts (Ts...) as an argument
     * @details ``Ts...`` must have at least two elements (a 1-element call is delegated to the T-template above). Internally, the line is converted into a ``std::tuple<Ts...>`` using ``JSL::String::ParseTo`` and ``std::apply`` used to unpack these into the function signature.
     * @param fileName The file to be read
     * @param delimiter The character sequence used to separate elements into the individual types of Ts... 
     * @param callback A function which is applied to each line, must accept (Ts...) as a function signature; i.e. <int,int,bool> must be a callable of type <void(int,int,bool)> 
     */
    template<typename... Ts, typename Func>
        requires std::invocable<Func, Ts...> && (sizeof...(Ts) > 1)
    void forConvertedLineIn(const std::filesystem::path fileName, std::string_view delimiter, Func callback)
    {
        forLineIn(fileName,[&](auto line)
        {
            std::tuple<Ts...> tmp = JSL::String::ParseTo<std::tuple<Ts...>>(line,delimiter);
            std::apply(callback,tmp);
        });
    }



}