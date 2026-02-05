#pragma once

#include <string>
#include <functional>
#include "../data/pipe.h"
#include "../modules/string.h"
#include "PerAxis.h"
#include "Colorbar.h"
#include "legend.h"

#define ADD(...) \
    pipe << __VA_ARGS__ << "\n";

#define IF_ELSE_ADD(condition, trueline, falseline) \
    do { if (condition) {ADD(trueline);} else {ADD(falseline);}} while(0)
#define IF_ADD(condition, trueline) \
    do { if (condition) {ADD(trueline);}} while(0)



namespace JSL::Plotting
{
    class Axis; //forward declaration for friend
    class Display
    {
        public:
            Properties::PerAxis X = Properties::PerAxis::Default("x");      
            Properties::PerAxis Y = Properties::PerAxis::Default("y"); 
            Properties::PerAxis Z = Properties::PerAxis::Dormant("z");    
            Color::Controller CB = Color::Controller();

            std::string Title;      
            bool ShowGrid = true;
            LegendObject Legend;
            std::vector<std::string> CustomLines;
        friend class JSL::Plotting::Axis; //allows the axis to call formatting functions, without exposing them to the API

            void CustomCommand(const std::string & cmd)
            {
                CustomLines.push_back(cmd);
            }
        private:
            void WriteFormatting(internal::GnuplotPipe & pipe)
            {
                //   some default values to be chucked in at the beginning
                pipe << "set border\n";
                pipe << "set boxwidth 0.8 relative\n";

                MemberToPipe(X,"x",pipe);
                MemberToPipe(Y,"y",pipe);
                MemberToPipe(Z,"z",pipe);
                MemberToPipe(CB,"cb",pipe);

                IF_ELSE_ADD(Title.size() > 0, 
                    "set title \"" << Title << "\"",
                    "unset title");
                IF_ELSE_ADD(ShowGrid,
                    "set grid", "unset grid");

                Legend.ToPipe(pipe);

                for (auto cmd : CustomLines)
                {
                    ADD(cmd);
                }
            }
            void MemberToPipe(Properties::PerAxis & axis, const std::string & axisName, internal::GnuplotPipe & pipe)
            {
                if (axis.Active)
                {
                    ADD("set " << axisName << "label \"" << axis.Label << "\"");
                    ADD("set " << axisName << "range " << axis.Range.ToString());
                    ADD("unset format " << axisName) //reset the formatting

                    axis.Tics.ToPipe(pipe,axisName,axis.TimeParsingFormat);

                    IF_ELSE_ADD(axis.Log,
                        "set logscale " << axisName,
                        "unset logscale " << axisName);
                    IF_ELSE_ADD(axis.Mirrored,
                        "set " << axisName << "tics mirror\nset " << axisName << "2tics\nset link " << axisName << "2 via " << axisName << " inverse " << axisName,
                        "set " << axisName << "tics nomirror");
                }
            }
    };
}

#undef ADD
#undef IF_ELSE_ADD
