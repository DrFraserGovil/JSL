#pragma once

#include <JSL/Parameters/NestedAggregator.h>
#include <JSL/Parameters/Parameter.h>



namespace JSL::Parameter 
{
    class Aggregator : public NestedAggregator
    {
        public:
        Aggregator(std::string_view name);
        void Parse(int argc, char** argv);

        Aggregator();

        //expose the protected command to public
        using NestedAggregator::Parse;

        std::vector<std::string> InvokedCommands();
        std::string & GetDefaultCommand();
        static std::map<std::string,std::string> & RegisteredCommands();
        std::pair<std::set<std::string>, std::set<std::string>> ParseCommands();
        void AddCommand(std::string name, std::string result);
        void DefaultCommand(std::string name, std::string result);
        protected:
            std::string Name = "Root";
            bool Help;
            Setting<bool> HelpToggle;
            void MainPrintHelp();
    };
}