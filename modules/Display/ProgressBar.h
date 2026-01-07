#include <string>
#include <vector>
#include "ANSI_Codes.h"
#include "../utils/jsl_error.h"
namespace JSL
{
    template<size_t Dimension>
    class ProgressBar
    {
        public:
           
            bool Animated;
            char Symbol;
           

            template<typename... Ts>
            requires (std::integral<Ts> && ...)
			ProgressBar(Ts... targetCounts) : Symbol('#')
			{
				static_assert( sizeof...(targetCounts)> 0);

				UnpackTargets(0,targetCounts...);
                SetWidth(20);
                SetAnimated(true);
                WrittenToScreen = false;
			}

            /*!
				Provide a progress update (i.e. loop index) for each of the active bars. Accepts a number of integer arguments equal to Dimension.
				\param positons The current process count (to be compared against the targets) of the open progress bars
			*/
			template<typename... Ts>
            void Update(Ts... progress)
            {
                static_assert(sizeof...(progress)==Dimension);
                UnpackProgress(0,progress...);
                CheckUpdates();
            }
            void Tick()
            {
                //cascading update from smallest to largest
                int head = Dimension;
                int summer = 0;
                do {
                    --head;
                    ++Progress[head];
                    if (Progress[head] == Targets[head])
                    {
                        Progress[head] = 0;
                    }
                    summer += Progress[head];
                } while (Progress[head] == 0 && head > 0);
                
                if (summer == 0)
                {
                    Progress = Targets;
                }
                CheckUpdates();
            }

            void SetWidth(size_t width)
            {
                Width = width;
                for (size_t i = 0; i < Dimension; ++i )
                {
                    WidthFraction[i] = width*1.0/(Targets[i]);
                }
            }
            void SetAnimated(bool value)
            {
                Animated = value;
            }

        private:
            size_t Width;
            std::array<size_t,Dimension> Targets;
            std::array<double,Dimension> WidthFraction;
            std::array<size_t,Dimension> Progress;
            std::array<size_t,Dimension> HashCount;
            bool WrittenToScreen;
            std::ostringstream buffer;
            void CheckUpdates()
            {
                bool requiresUpdates = false;
                int addHashes = 0;
                size_t upperLimit = Dimension;
                if (!Animated) {upperLimit = 1;};
                for (size_t i =0; i < upperLimit; ++i )
                {
                    int newHashes = (int)( WidthFraction[i] * Progress[i] + 0.499);
                    if (newHashes != HashCount[i])
                    {
                        requiresUpdates = true;
                        if (i == 0)
                        {
                            addHashes = newHashes - HashCount[i];
                        }
                        HashCount[i] = newHashes;
                    }
                }

                if (requiresUpdates && Animated)
                {
                    AnimatedUpdate();
                }
                if (requiresUpdates && !Animated)
                {
                    StaticUpdate(addHashes);
                }
            }

            void AnimatedUpdate()
            {
                buffer.clear();
                buffer.str("");
                if (WrittenToScreen)
                {
                    buffer << JSL::Cursor::Move(Cursor::Direction::Up,Dimension);
                    buffer << JSL::Cursor::Move(Cursor::Direction::Left,1000);//move all the way left
                }
                else
                {
                    WrittenToScreen = true;
                }
                
                for (size_t i = 0; i < Dimension; ++i)
                {
                    buffer << "[" << std::string(HashCount[i],Symbol) << std::string(Width-HashCount[i],' ') << "]\n";
                }
                std::cout << JSL::Cursor::Hide << buffer.str() << JSL::Cursor::Show;
               
            }
            void StaticUpdate(size_t newHashes)
            {
                
                buffer.clear();
                buffer.str("");
                // //when static, even if multidimensional, only plot the
                if (!WrittenToScreen)
                {
                    buffer << "[";
                    WrittenToScreen=true;
                }
                buffer << std::string(newHashes,Symbol);
                if (HashCount[0] == Width)
                {
                    buffer << "]\n";
                }
                std::cout << buffer.str() <<std::flush;
                
            }
            
            template<typename... Ts>
            void UnpackProgress(int idx, int progress, Ts... remainder)
            {
                Progress[idx] = std::min(progress,Targets[idx]);
                if constexpr (sizeof...(remainder) > 0)
                {
                    UnpackProgress(idx+1,remainder...);
                }
            }
            //! The variadic internal function called by the constructor, looping over the arbitrary number of inputs and storing them in internal vectors
            template<typename... Ts>
			void UnpackTargets(int idx, int target, Ts... remainder)
			{
				Targets[idx] = target;
                Progress[idx] = 0;
                HashCount[idx] = 0;
                if constexpr (sizeof...(remainder) > 0)
				{
					UnpackTargets(idx+1,remainder...);
				}

			}
    };

    // Defaulting Symbol to '#'
    template <typename... Ts>
    ProgressBar(Ts...) -> ProgressBar<sizeof...(Ts)>;

}