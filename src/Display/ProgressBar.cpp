#include <JSL/Display/ProgressBar.h>
#include <JSL/Display/ANSI_Codes.h>
#include <JSL/Display/Log.h>
#include <sstream>

namespace JSL::Display::Progress
{
    namespace internal
    {
        

        ProgressEngine::ProgressEngine(std::vector<size_t> && lengths) : Dimension(lengths), Depth(lengths.size())
        {
            Progress = std::vector<size_t>(Depth,0);
            MarkCount = std::vector<size_t>(Depth,0);
        }

        void ProgressEngine::Tick()
        {
            if (Finished) return;
            int idx = Depth-1;
            while (idx >= 0)
            {
                ++Progress[idx];
                if (Progress[idx] < Dimension[idx])
                {
                    CheckMarks(idx);
                    return;
                }
                else
                {
                    Progress[idx] = 0;
                    --idx;
                }
            }

            Finished = true;
            Progress = Dimension;
            CheckMarks(0);
        }

        void ProgressEngine::CheckMarks(int idx)
        {
            bool needsRender = false;
            for (size_t i = idx; i < Depth; ++i)
            {
                size_t expected = std::min(RenderLength, (size_t)(0.5 + (Progress[i] * RenderLength) * 1.0/Dimension[i])); //old fashioned rounding
                if (expected != MarkCount[i])
                {
                    MarkCount[i] = expected;
                    needsRender = true;
                }
            }
            if (needsRender)
            {
                Render();
            }
        }
    }


    ////Animated bar
    Bar::Bar(size_t length,size_t barlength) : ProgressEngine({length})
    {
        RenderLength = barlength;
        Render();
    }
    Bar::Bar(std::vector<size_t>&&  lengths,size_t barlength) : ProgressEngine(std::move(lengths))
    {
        RenderLength = barlength;
        Render();
    }

    void Bar::Render()
    {
        std::ostringstream buffer;
        if (hasRendered)
        {
            buffer << JSL::Terminal::Move(Terminal::Direction::Up,Depth);
            buffer << JSL::Terminal::MoveToColumn(0);//move all the way left
        }
        hasRendered = true;
        for (size_t i = 0; i < Depth; ++i)
        {
            buffer << "[" << std::string(MarkCount[i],Symbol) << std::string(RenderLength-MarkCount[i],' ') << "] " << "\n";
        }
        std::cout << buffer.str() << std::flush;
    }



    ///static bar
    Static::Static(size_t length,size_t barlength) : ProgressEngine({length})
    {
        RenderLength = barlength;
    }

    void Static::Render()
    {
        std::ostringstream buffer;
        if (LastRender == 0)
        {
            buffer << "[";
        }
        int newmarks = (MarkCount[0] - LastRender);
        buffer << std::string(newmarks,Symbol);
        if (Finished)
        {
            buffer <<"]\n";
        }
        LastRender = MarkCount[0];
        std::cout << buffer.str() << std::flush;
    }
}