#pragma once

#include <iostream>
#include "curve.h"
#include <algorithm>
#include "pipe.h"
#include "visuals.h"
namespace JSL::Plotting
{
    template <typename T>
    using Vec = const std::vector<T>&;

    namespace AxisMembers
    {
        
        class Data
        {
            private:
                Display & Visual;  
            public:
                Data(Display & disp) : Visual(disp){};
                std::vector<Curve> Curves; //generic name for any entity which ends up being plotted
                template<class T, class S>
                void Plot(Vec<T> x, Vec<S> y)
                {
                    Curves.emplace_back(Curve(x,y));
                }
        };

        
    }

    class Axis
    {
        AxisMembers::Display S;
        AxisMembers::Data D;
        int Row;
        int Column;
        
        public:
            bool IsDirty;
            Axis() : D(S){
                IsDirty = true;
            }
            AxisMembers::Data & Add()
            {
                return D;
            }

            AxisMembers::Display & Set()
            {
                return S;
            }

            void Clear()
            {
                D.Curves.resize(0);
            }

            void Show(internal::GnuplotPipe & pipe)
            {
                pipe << "# Begin axis (" << Row << ", " << Column << ")\n";
                // pipe << "unset key;";

                S.WriteFormatting(pipe);

                //specify the curves first
                for (size_t i = 0; i < D.Curves.size(); ++i)
                {
                    if (i > 0)
                    {
                        pipe << ", \\\n\t";
                    }
                    else
                    {
                        pipe << "\nplot ";
                    }
                    pipe << "$Data_Axis" << Row <<"x" << Column <<"_Curve" << i <<" with lines";
                }
                pipe << "\n";
            }

            void SaveData(internal::GnuplotPipe & pipe, int myRow, int myCol)
            {

                for (size_t i = 0; i < D.Curves.size(); ++i)
                {
                    D.Curves[i].SaveData(pipe,myRow,myCol,i,IsDirty);
                }
                Row = myRow;
                Column = myCol;
                IsDirty = false;
            }
    };
} // namespace JSL
