#pragma once

#include <string>
#include "../../Strings/Strings.h"
namespace JSL::Plotting
{
    template <typename T>
    using Vec = const std::vector<T>&;

    class Curve
    {
        

        public:
            Curve(){
                X.resize(0);
                Y.resize(0);
                Z.resize(0);
            };
            template<class T, class S>
            Curve(Vec<T> x, Vec<S> y)
            {
                size_t n = x.size();
                CurveAssert("x and y vectors must be of equal length",n==y.size());
                
                X.resize(n);
                Y.resize(n);
                Z.resize(0);
                for (size_t i =0; i < n; ++i)
                {
                    X[i] = MakeString(x[i]);
                    Y[i] = MakeString(y[i]);
                }
            }

            void SaveData(internal::GnuplotPipe & pipe, int row, int col, int id, bool forceWrite)
            {
                bool alreadyWritten =  (row==Row) && (col == Column) &&(ID == id);
                if (forceWrite || !alreadyWritten)
                {
                    pipe << "$Data_Axis" <<row <<"x" << col <<"_Curve" << id << " << EOD\n";
                
                    for (size_t i = 0; i < X.size(); ++i)
                    {
                        pipe << X[i] << " " << Y[i];
                        if (Z.size() > 0)
                        {
                            pipe << " " << Z[i];
                        }
                        pipe << "\n";
                    }
                    pipe << "EOD\n\n";
                    Row = row;
                    ID = id;
                    Column = col;
                }
            }
            std::vector<std::string> X;
            std::vector<std::string> Y;
            std::vector<std::string> Z;
        private:
            int Row =-1;
            int Column =-1;
            int ID = -1;
            void CurveAssert(const std::string & message, bool assertion)
            {
                if (!assertion)
                {
                    std::cout << message << std::endl;
                    exit(1);
                }
            }
    };
}