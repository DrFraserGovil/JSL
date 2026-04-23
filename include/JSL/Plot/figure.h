#pragma once
#include <vector>
#include "data/pipe.h"
#include "axis.h"
#include <sstream>
namespace JSL::Plotting
{
    using szt = size_t;

    
    class Figure
    {
        private: //members
            std::vector<std::vector<Plotting::Axis>> axis;
            szt active_row = 0;
            szt active_col = 0;
            internal::GnuplotPipe pipe;
            szt Rows;
            szt Columns;

        public:
            double ScriptFramePause = 0.05;

        Figure()
        {
            SetMultiplot(1);
        }

        void Persist(unsigned int level)
        {
            if (level >2)
            {
                throw std::logic_error("Persistence ids must be in [0,1,2].");
            }
            Persist(static_cast<Output>(level));
        }
        void Persist(Output level)
        {
            pipe.SetPersistence(level);
        }
        void ScriptPath(std::string path)
        {
            pipe.SetScriptPath(path);
        }
        void ClearAll()
        {
            for (szt r = 0; r < Rows; ++r)
            {
                for (szt c = 0; c < Columns; ++c)
                {
                   axis[r][c].Clear();
                }
            }
        }
        void Clear()
        {
            Clear(active_row,active_col);
        }
        void Clear(int id)
        {
            auto [row,col] = IdToCoord(id);
            Clear(row,col);
        }
        void Clear(int row, int col)
        {
            axis[row][col].Clear();
        }

        void Show()
        {
            pipe << "# New show called\n";
            for (szt r = 0; r < Rows; ++r)
            {
                for (szt c = 0; c < Columns; ++c)
                {
                    axis[r][c].SaveData(pipe,r,c);
                }
            }
            MultiplotPrint();
            for (szt r = 0; r < Rows; ++r)
            {
                for (szt c = 0; c < Columns; ++c)
                {
                   axis[r][c].Show(pipe);
                }
                pipe << "\n";
            }
            if (Rows *Columns > 1)
            {
                pipe << "unset multiplot\n";
            }
            if (pipe.GetMode() == Output::Scripted)
            {
                pipe << "pause " << ScriptFramePause <<"\n";
            }
            pipe.flush();
        }
        void Close()
        {
            pipe.Close();
        }

        void SetMultiplot(size_t rowCount, size_t columnCount=1)
        {
            //size 0 is meaningless; so buffer up
            rowCount = std::max((size_t)1,rowCount);
            columnCount = std::max((size_t)1,columnCount);

            //resize the internal arrays, leaving the existing data intact
            axis.resize(rowCount);
            for (szt j = 0; j < rowCount; ++j)
            {
                axis[j].resize(columnCount);
                for (szt k = 0; k < rowCount; ++k)
                {
                    axis[j][k].IsDirty = true; // touch it and make it dirty
                }
            }
            Rows = rowCount;
            Columns = columnCount;
        }

        void MoveTo(size_t id)
        {
            auto [row,col] = IdToCoord(id);
            MoveTo(row,col);
        }
        void MoveTo(size_t row, size_t col)
        {
            CheckAxisBounds(row,col);
            active_col = col;
            active_row = row;
        }
        std::vector<Axis> & operator[](size_t row)
        {
            CheckAxisBounds(row,0);
            return axis[row];
        }


        Plotting::Axis& Active() {  return axis[active_row][active_col]; }
        Plotting::AxisMembers::Data &     Add() { return Active().Add(); };
        Plotting::Display &  Set() { return Active().Set(); };

        private:
            void CheckAxisBounds(size_t row, size_t col)
            {
                if (row >= Rows)
                {
                    throw std::runtime_error("Cannot access row "       + std::to_string(row) + ": out of bounds.");
                }
                if (col >= Columns)
                {
                    throw std::runtime_error("Cannot access column "    + std::to_string(row) + ": out of bounds.");
                }
            }

            void MultiplotPrint()
            {
                if (Rows * Columns > 1)
                {
                    pipe << "set multiplot layout ";
                    pipe << Rows << ", " << Columns << "\n";
                    pipe.flush();
                }
            }

            std::pair<int,int> IdToCoord(szt id)
            {
                if (id >= Columns * Rows)
                {
                    throw std::runtime_error("Cannot access axis " + std::to_string(id) + ": out of bounds.");
                }
                int col = id % Columns;
                int row = (id - col)/Columns;
                return {row,col};
            }
    };
}