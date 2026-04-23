#pragma once

#include <string>

#define ADD(...) \
    pipe << __VA_ARGS__ << "\n";

#define IF_ELSE_ADD(condition, trueline, falseline) \
    do { if (condition) {ADD(trueline);} else {ADD(falseline);}} while(0)
#define IF_ADD(condition, trueline) \
    do { if (condition) {ADD(trueline);}} while(0)

namespace JSL::Plotting
{
    enum class TicType
    {
        Default,
        Time,
        Power,
    };

    enum class TicDirection
    {
        In,
        Out
    };

    class TicLabel
    {
        public:
            std::string Name;
            double Position;
            bool AssignedPos;
            
            template<class T>
            TicLabel(const T & nameOnly): Name(MakeString(nameOnly))
            {
                AssignedPos = false;
            }
            template<class T>
            TicLabel(const T & name, double pos) : Name(MakeString(name)), Position(pos)
            {
                AssignedPos = true;
            }
            void ToPipe(JSL::Plotting::internal::GnuplotPipe & pipe, int id)
            {
                pipe << "\"" << Name << "\" " << (AssignedPos ? Position : id) << ", ";
            }
    };

    class Display;
    class TicProperties
    {
        public:
            unsigned int Rotate = 0;
            double Interval = 0;
            TicType Format = TicType::Default;
            std::string TimeFormat = "%H:%M:%S";
            std::vector<TicLabel> Labels;

            void FormatAsTime()
            {
                Format = TicType::Time;
            }
            void FormatAsPower()
            {
                Format = TicType::Power;
            }
            friend class JSL::Plotting::Display;
        private:
            
            void ToPipe(internal::GnuplotPipe & pipe,const std::string & axisName,const std::string & timeParse)
            {
                switch (Format)
                {
                    case TicType::Default:
                        ADD("set " << axisName << "data");
                        break;
                    case TicType::Power:
                        ADD("set " << axisName << "data\n"<<
                            "set format " << axisName << "\"10^{%T}\"");
                        break;
                    case TicType::Time:
                        ADD("set " << axisName << "data time\n"<<
                            "set timefmt \"" << timeParse << "\"\n" <<
                            "set format " << axisName << " \"" << TimeFormat << "\""
                            );

                }

                IF_ELSE_ADD(Rotate > 0,
                        "set " << axisName << "tics rotate by " << Rotate << " offset " << -2 * std::cos(3.141592654/180 * Rotate)<< "," << -2* std::sin(3.141592654/180 * Rotate),
                        "set " << axisName << "tics rotate by 0 offset 0,0"
                );

                IF_ELSE_ADD(Interval != 0,
                    "set " << axisName << "tics " << Interval,
                    "set " << axisName << "tics autofreq"
                );

                IF_ADD(Labels.size() > 0,
                    "set " << axisName << "tics (";
                    for (size_t i = 0; i < Labels.size(); ++i)
                    {
                        Labels[i].ToPipe(pipe,i);
                    }
                    pipe << ")"
                );
            }
    };
}

#undef ADD
#undef IF_ELSE_ADD
