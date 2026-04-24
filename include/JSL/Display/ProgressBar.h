#pragma once
#include <stdio.h>
#include <vector>
namespace JSL::Progress
{
    namespace internal
    {
        class ProgressEngine
        {
            public:
                char Symbol = '#';
                const std::vector<size_t> Dimension;
                const size_t Depth;
                ProgressEngine(std::vector<size_t> && Dimensions);
                void Tick();
                void Update(std::vector<int> && positions);
                void Update(int position);
            protected:
                void CheckMarks(int idx);
                std::vector<size_t> MarkCount;
                std::vector<size_t> Progress;
                virtual void Render(){};
                size_t RenderLength;
                bool Finished = false;
        };
    }
    class Bar : public internal::ProgressEngine
    {
        public:
            
            Bar(size_t duration, size_t barLength=20);
            Bar(std::vector<size_t> && durations, size_t barLength=20);
        private:
            void Render();
            bool hasRendered = false;
    };

    class Static : public internal::ProgressEngine
    {
        public:
            Static(size_t length, size_t barLength = 20);
        private:
            void Render();
            int LastRender=0;
    };
};

