#pragma once

#include <JSL/Parameters/Aggregator.h>
#include <JSL/Parameters/Parameter.h>



namespace JSL::Parameter 
{
    class RootAggregator : public Aggregator
    {
        public:
        RootAggregator(std::string_view name);
        void Parse(int argc, char** argv);

        RootAggregator();
            
        protected:
            std::string Name = "Root";
            bool Help;
            Setting<bool> HelpToggle;
            void MainPrintHelp();
    };
}