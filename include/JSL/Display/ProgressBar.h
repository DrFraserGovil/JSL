#pragma once
#include <stdio.h>
#include <vector>
namespace JSL::Display::Progress
{
    namespace internal
    {
        /// @brief The core interface for progress bars
        class ProgressEngine
        {
            public:
                /// @brief The symbol which is used to fill up the bar
                char Symbol = '#';

                /// @brief The sizes (i.e. how many ticks to completion) for each of the sub-bars
                /// @details Always has size() = Depth
                const std::vector<size_t> Length;

                /// @brief How many nested bars are present.
                /// @details For Static bars, this is always 1, but Dynamic bars can nest (i.e. to track for(i < 10); for(j < 20))
                const size_t Depth;

                /// @brief Constructor which fixes the amount of work which needs to be done to complete the progress bar
                /// @param lengths A vector of 'ticks needed' for each sub-bar. Depth is inferred from the length of this vector. 
                ProgressEngine(std::vector<size_t> && lengths);

                /// @brief Increments the progress of the deepest bar by one tick. If this fills the bar, cascades up, adding a tick to the next bar up (and so on).
                void Tick();

                /// @brief Fix the values of all the bars to the provided positions
                /// @tparam T a type convertible to size_t
                /// @param positions A vector of positions (in tick-counts) to set each sub-bar to
                /// @throws runtime_error if the length of input does not match the depth
                template<class T>
                void Update(std::vector<T> && positions)
                {
                    if (positions.size()!= Depth)
                    {
                        ThrowSizeMismatch(positions.size()); 
                    }
                    for (size_t i = 0; i < Depth; ++i)
                    {
                        Progress[i] = static_cast<size_t>(positions[i]);
                    }
                    CheckMarks(0);
                }

                /// @brief Fix the value of the deepest bar to the provided position 
                /// @param position A position (in tick-counts) to set the lowest-level bar to
                void Update(size_t position);
            protected:
                /// @brief Checks the progress status after an update and determines if a rerender is needed
                /// @param idx The index of the highest-priority bar that was last modified (only bars lower priority (higher index) than this are checked) 
                void CheckMarks(int idx);

                /// @brief The current number of marks which have been rendered to screen for each sub-bar
                std::vector<size_t> MarkCount;
                /// @brief The current number of ticks which each sub-bar has accumulated
                std::vector<size_t> Progress;
                 /// @brief A pure virtual function for rendering to screen; this is what we delegate to the child classes
                virtual void Render()=0;

                /// @brief The size (in characters) of a full bar
                size_t RenderLength=20;
                /// @brief True when the bar has reached 100% progress, subsquent render calls are ignored
                bool Finished = false;
                /// @brief Helper function to throw errors without exposing the jsl error api
                /// @param actual The size of the input position vector (should be Depth, but if you're here, it's not!) 
                void ThrowSizeMismatch(size_t actual);
        };
    }

    /// @brief A dynamic progress bar which uses Terminal::ClearLine commands to erase the previous bar, and write a new one with the updated progress. Allows for multiple nested bars, and for both forward and backward progress. 
    class Bar : public internal::ProgressEngine
    {
        public:
            /// @brief Constructor for a single-layer dynamic progress bar
            /// @param duration The number of ticks needed to fill the bar
            /// @param barLength The number of characters in a full bar (i.e. the resolution of the bar)
            Bar(size_t duration, size_t barLength=20);


            /// @brief Constructor for a multi-layer dynamic progress bar
            /// @param durations A vector of length N (assigned to Depth) containing the number of ticks needed to fill each bar, starting from the outermost nest, to the innermost. 
            /// @param barLength The number of characters in a full bar (i.e. the resolution of the bar)
            Bar(std::vector<size_t> && durations, size_t barLength=20);
        private:
            /// @brief Key rendering function; clears the previous line (if present), resets the cursor, and then re-writes the current progress bar
            void Render();
            /// @brief Boolean tag to track if there is content which needs to be deleted before a rerender
            bool hasRendered = false;
    };
    /// @brief A static progress bar which slowly outputs one character at a time, until the bar is full, with no deletions or cursor magic. Ideal for file-streams where ANSI escape sequences would result in a horrible mess. As a result, cannot be nested.
    class Static : public internal::ProgressEngine
    {
        public:
            /// @brief Constructor for a single-layer static progress bar
            /// @param length The number of ticks needed to fill the bar
             /// @param barLength The number of characters in a full bar (i.e. the resolution of the bar)
            Static(size_t length, size_t barLength = 20);
        private:
            /// @brief Key rendering function; computes the number of new marks needing to be written, and pushes them to the stream
            void Render();

            /// @brief The current number of marks written to screen
            int LastRender=0;
    };
};

