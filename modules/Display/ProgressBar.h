#include <string>
#include <vector>
#include "ANSI_Codes.h"
#include "../utils/jsl_error.h"
#include "../Time/Timer.h"

namespace JSL
{
    template<size_t Dimension>
    class ProgressBar
    {
        public:
           
            bool Animated;
            char Symbol;
            double AnimationDelay = 0.02;   

            template<typename... Ts>
			ProgressBar(Ts... targetCounts) : Symbol('#')
			{
                static_assert((std::is_integral_v<Ts> && ...),"ProgressBar targets must be integral types (int, size_t, etc.)");
				static_assert( sizeof...(targetCounts)> 0,"Number of arguments must match the ProgressBar Dimension!");

				UnpackTargets(0,targetCounts...);
                SetWidth(25);
                SetAnimated(true);
                WrittenToScreen = false;
                T.Start();
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
                    if (Progress[head] >= Targets[head])
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

            void SetPrefix(size_t pos, const std::string & prefix)
            {
                if (pos >= Dimension)
                {
                    internal::FatalError("Out of range error (progress bar)") << "Cannot assign prefix " << pos << " (" << prefix <<") to progress bar of dimension " << Dimension; 
                }
                Prefix[pos] = prefix;
                BufferPrefix();
            }
            void SetSuffix(size_t pos, const std::string & prefix)
            {
                if (pos >= Dimension)
                {
                    internal::FatalError("Out of range error (progress bar)") << "Cannot assign suffix " << pos << " (" << prefix <<") to progress bar of dimension " << Dimension; 
                }
                Suffix[pos] = prefix;
            }
            
            template<typename... Ts>
            void Suffixes(Ts... strings)
            {
                static_assert(sizeof...(strings)==Dimension);
                UnpackStrings(Suffix,0,strings...);
            }
            template<typename... Ts>
            void Prefixes(Ts... strings)
            {
                static_assert(sizeof...(strings)==Dimension);
                UnpackStrings(Prefix,0,strings...);
                BufferPrefix();
                std::cout << "Set prefixes" << std::endl;
            }
        private:
            size_t Width;
            std::array<size_t,Dimension> Targets;
            std::array<double,Dimension> WidthFraction;
            std::array<size_t,Dimension> Progress;
            std::array<size_t,Dimension> HashCount;
            std::array<std::string,Dimension> Prefix;
            std::array<std::string,Dimension> Suffix;
            Timer T;
            bool WrittenToScreen;
            bool alreadyFull = false;
            std::ostringstream buffer;
            void CheckUpdates()
            {
                bool requiresUpdates = !WrittenToScreen;
                int addHashes = 0;
                size_t upperLimit = Dimension;
                bool full = true;
                if (!Animated) {upperLimit = 1;};
                for (size_t i =0; i < upperLimit; ++i )
                {
                    if (Progress[i] < Targets[i]-1)
                    {
                        full = false;
                    }
                    size_t newHashes = std::min((int)Width,(int)( WidthFraction[i] * Progress[i] + 0.499));
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
                if (alreadyFull)
                {
                    full = false;
                }
                if (full || (requiresUpdates))
                {
                    if (full)
                    {
                        alreadyFull = true;
                    }
                    TriggerUpdate(addHashes,full);
                   
                }
            }
            void TriggerUpdate(int addHashes,bool forceUpdate)
            {
                if (Animated)
                {
                    if (T.Lap() > AnimationDelay || forceUpdate)
                    {
                        AnimatedUpdate();
                        T.Start();
                    }

                }
                else
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
                    buffer << Prefix[i] << "[" << std::string(HashCount[i],Symbol) << std::string(Width-HashCount[i],' ') << "] " << Suffix[i] << "\n";
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
                    buffer << Prefix[0] << "[";
                    WrittenToScreen=true;
                }
                buffer << std::string(newHashes,Symbol);
                if (HashCount[0] == Width and not alreadyFull)
                {
                    buffer << "]\n";
                }
                std::cout << buffer.str() <<std::flush;
                
            }
            
            void BufferPrefix()
            {
                int maxPos = 0;
                size_t maxSize = Prefix[0].length();
                for (size_t i = 1; i < Prefix.size(); ++i)
                {
                    size_t testSize = Prefix[i].length();
                    if (testSize > maxSize)
                    {
                        maxSize = testSize;
                        maxPos = i;
                    }
                }
                std::string & largest = Prefix[maxPos];
                char finalChar = largest[largest.length() - 1];
                if (finalChar != ' ')
                {
                    largest += " ";
                    maxSize += 1;
                }
                for (auto & prefix : Prefix)
                {
                    prefix += std::string(maxSize-prefix.size(),' ');
                }
            }

            template<typename... Ts>
            void UnpackProgress(int idx, int progress, Ts... remainder)
            {
                Progress[idx] = std::min((int)progress,(int)Targets[idx]);
                if constexpr (sizeof...(remainder) > 0)
                {
                    UnpackProgress(idx+1,remainder...);
                }
            }
            //! The variadic internal function called by the constructor, looping over the arbitrary number of inputs and storing them in internal vectors
            template<typename... Ts>
			void UnpackTargets(int idx, int target, Ts... remainder)
			{
                if (target == 0)
                {
                    internal::FatalError("Zero-size progress bar") << "Dimension " << idx << " of the progress bar is 0";
                }
				Targets[idx] = target;
                Suffix[idx] = "";
                Prefix[idx] = "";
                Progress[idx] = 0;
                HashCount[idx] = 0;
                if constexpr (sizeof...(remainder) > 0)
				{
					UnpackTargets(idx+1,remainder...);
				}

			}

            template<typename... Ts>
			void UnpackStrings(std::array<std::string,Dimension> & targetArray, int idx, std::string target, Ts... remainder)
			{
                targetArray[idx] = target;
                if constexpr (sizeof...(remainder) > 0)
				{
					UnpackStrings(targetArray,idx+1,remainder...);
				}
            }
    };

    template<typename ... Ts>
    ProgressBar(Ts...) -> ProgressBar<sizeof...(Ts)>;

}