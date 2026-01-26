#pragma once

#include <vector>
#include <cstring>
#include <cctype>
#include "../Strings/Strings.h"
namespace JSL
{
    namespace internal
    {
        const std::string NULLFILE= "__none__";

        //some help metafunctions to help identify vector types
        template <typename T>
        struct is_vector : std::false_type {};
        template <typename U>
        struct is_vector<std::vector<U>> : std::true_type {};

        bool inline ElementIsValue(char * nextElement)
        {
            bool hasDash = (nextElement[0] == '-'); //dashes signify commands, but also negative nos.
            bool isSingleCharacter = (std::strlen(nextElement) == 1);

            if (!hasDash || isSingleCharacter)
            {
                return true; //entries that are not preceeded by a dash, or are a single character long, cannot be command triggers
            }

            return isdigit(nextElement[1]); //detect if the next string is a number, if so return true. This is the case of a negative no.
            
        }



        struct HelpMessages
        {
            std::string Name;
            std::vector<std::tuple<std::string,std::string,std::string,std::string>> Messages;

            template<class T>
            void AddMessage(std::string flag,T defaultValue,std::string name,std::string description)
            {
                Messages.push_back(std::make_tuple(flag,name,description,MakeString(defaultValue)));
                // Messages.push_back(std::make_tuple<std::string,std::string,std::string,std::string>(flag,"","",MakeString(defaultValue)));	
            }

            void scanSizes(std::pair<int,int> & lengths)
            {
                for (auto message : Messages)
                {
                    int keyLength = std::get<0>(message).size();
                    int nameLength = std::get<1>(message).size();

                    if (keyLength > lengths.first)
                    {
                        lengths.first = keyLength;
                    }
                    if (nameLength > lengths.second)
                    {
                        lengths.second = nameLength;
                    }
                }
            }

            void print(std::pair<int,int> & lengths)
            {
                int maxKeyLength = lengths.first;
                int maxNameLength = lengths.second;
                

                std::cout << Name << ":\n\n";

                sort(Messages.begin(), Messages.end(),
                [](const std::tuple<std::string,std::string,std::string,std::string>& a,
                    const std::tuple<std::string,std::string,std::string,std::string>& b) {
                    return std::get<0>(a) < std::get<0>(b);
                });

                
                int bufferSpace = 4;
                int minSize = 10;
                int nameStartPos = std::max(minSize,maxKeyLength + bufferSpace);
                int descStartPos =  std::max(minSize,maxNameLength + bufferSpace);
                auto newLineBuffer =  std::string(3+nameStartPos+descStartPos,' ');
                for (auto message : Messages)
                {
                    auto key = std::get<0>(message);
                    auto name = std::get<1>(message);
                    auto keyNameGap =  std::string(nameStartPos-key.size(),' '); 
                    auto nameDescGap = std::string(descStartPos-name.size(),' ');
                    std::cout << "  -" << key;
                    std::cout << keyNameGap << name;

                    auto descLines = split(std::get<2>(message),"\n");
                    std::cout << nameDescGap  << descLines[0] << "\n";
                    for (size_t i = 1; i < descLines.size(); ++i)
                    {
                        std::cout << newLineBuffer << descLines[i] << "\n";
                    }

                    std::cout << newLineBuffer << "default(" << std::get<3>(message) << ")\n"; 
                }
            }
        };

    } // namespace internal
    
}