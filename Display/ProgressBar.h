#pragma once

#include <string>
#include <vector>
#include <iomanip>
#include "clearScreen.h"
namespace JSL
{
	/*!
		A neat display widget for tracking the progress of a process by printing a progress bar on the screen, which updates to visually indicate the process is running. Supports arbitrary "levels" of progress bars which can be independently modified through a variadic argument interface.
		\tparam Dimension The number of concurrent bars to print (fixes the number of arguments which must be provided to subsequent functions). Defaults to 1 if 0 template arguments provided
		\tparam DeleteMode If true, utilises JSL::deleteLine() to remove the previous line before reprinting, such that the bar appears animated. If false, provides a more limited form of animation (false only available in Dimension==1 mode). Defaults to true.
		\tparam Symbol The symbol which is printed within the bar to fill up space. Defaults to "#".
		\tparam MaxHashes The number of symbols in a full bar (dictates the width of the progress bar)
	*/
	template<int Dimension = 1, bool DeleteMode = true, char Symbol = '#',unsigned int MaxHashes =20>
	class ProgressBar
	{
		private:
			bool firstPrint = true; //!<Toggle to ensure open-brace "[" is printed only once
			std::vector<int> BarTargets; //!<Storage for the target number of processes
			std::vector<double> BarProgress; //!< Current progress value (between 0 and 1)
			std::vector<int> Hashes; //!< Current printed number of symbols for each bar
			std::vector<std::string> Names; //!< The names printed in front of each bar 

			bool reprintNeeded; //!<Toggle which ensures reprinting only occurs when something changes, prevents unnecessary overhead
			int bufferWidth = 10; //!<Default size allocated for names, increases if a value of anmes would overfill it
			int prevHashes = 0; //!<Used in DeleteMode=false to compute new hashe number

			//! The variadic internal function called by the compiler, looping over the arbitrary number of inputs and storing them in internal vectors
			template<typename... Ts>
			void UnpackTargets(int target, Ts... targets)
			{
				
				BarTargets.push_back(target);
				BarProgress.push_back(0);
				Hashes.push_back(0);
				if constexpr (sizeof...(targets) > 0)
				{
					UnpackTargets(targets...);
				}
			}

			//! The variadic internal function which is called by Update(), loops over the arguments and computes the number of Symbols required by each bar
			template<typename... Ts>
			void UnrollPositions(const int idx,int pos, Ts... remainder)
			{
				BarProgress[idx]  = (double)pos/(BarTargets[idx]-1);
				int newHashes = std::min(BarProgress[idx],1.0) * MaxHashes;
				
				if (newHashes != Hashes[idx])
				{
					prevHashes = Hashes[idx];
					reprintNeeded = true;
					Hashes[idx] = newHashes;
				}
				if constexpr( sizeof...(remainder)>0)
				{
					UnrollPositions(idx+1,remainder...);
				}
				
				
			}

			//! Loops over the existing bars and erases them using ANSI codes
			void ClearScreen()
			{
				for (int i = 0; i < Dimension; ++i)
				{
					jumpLineUp();
					deleteLine();
				}
			}
			//! The version of Printing which happens if DeleteMode is true -- clears the previous bars using ANSI codes, then overwrites them with the new values
			void PrintPositions_DeleteMode()
			{
				if (!firstPrint)
				{
					ClearScreen();
				}
				firstPrint = false;
				for (int i = 0; i < Dimension; ++i)
				{
					int newHashes = Hashes[i];
					if (Names.size() > 0)
					{
						std::cout << std::setw(bufferWidth)  << Names[i]<< " ";
					}
					std::cout << "[" << std::string(newHashes,Symbol) << std::string(MaxHashes - newHashes,' ') << "]\n";
				}
			}

			//! The version of Printing which happens if DeleteMode is false - gradually prints the progress bar one Symbol at a time, then adds the close brace "]" whn the final one is printed
			void PrintPositions_RetainMode()
			{
		
				int newHashes = Hashes[0] - prevHashes;
				if (prevHashes == 0)
				{
					if (Names.size() == 1)
					{
						std::cout << std::setw(bufferWidth)  << Names[0]<< " ";
					}
					std::cout << "[";
				}
				std::cout << std::string(newHashes,Symbol);
				if (Hashes[0] == MaxHashes)
				{
					std::cout << "]";
				}
				std::cout << std::flush;
			}
		public:

			/*!
				Constructor function. Accepts a number of integer arguments equal to Dimension, throws an error if this is not true.
				\param targetCounts The number of processes (i.e. max loop index) that must complete before each bar is considered full.
			*/
			template<typename... Ts>
			ProgressBar(Ts... targetCounts) 
			{
				static_assert(sizeof...(targetCounts) == Dimension);
				static_assert(Dimension > 0);
				static_assert(MaxHashes > 1);
				UnpackTargets(targetCounts...);
			}

			/*!
				Provide a progress update (i.e. loop index) for each of the active bars. Accepts a number of integer arguments equal to Dimension.
				\param positons The current process count (to be compared against the targets) of the open progress bars
			*/
			template<typename... Ts>
			void Update(Ts... positions)
			{
				static_assert(sizeof...(positions) == Dimension);
				reprintNeeded  = false;
				UnrollPositions(0,positions...);

				//only reprint the bar if something has changed
				if (reprintNeeded)
				{
					if constexpr (DeleteMode)
					{
						PrintPositions_DeleteMode();
					}
					else
					{
						PrintPositions_RetainMode();
					}
				}
			}

			

			//! Sets all names for the bars simultaneously \param names A vector of strings (equal to Dimension), each of which is printed before their respective bar.
			void SetName(const std::vector<std::string> & names)
			{
				Assert("Name vector must be same size as bar count",names.size() == Dimension);
				Names = names;
				for (int i = 0; i < Dimension; ++i)
				{
					bufferWidth = std::max(bufferWidth,(int)names[i].length());
				}
			}

			//! Changes the name of the idx-th bar \param idx The bar name to be changes \param name The string which is printed in the front of the bar
			void SetName(unsigned int idx, const std::string & name)
			{
				Assert("Assign name to bar between 0 and BarCount-1", idx < Dimension);
				if (Names.size() < Dimension)
				{
					Names = std::vector<std::string>(Dimension,"");
				}
				Names[idx] = name;
				bufferWidth = std::max(bufferWidth,(int)name.length());
			}
			
			//!An override for SetName(unsigned int idx, const std::string & name) available only when Dimension == 1 \param name The message to go in front of the bar as it prints
			void SetName(const std::string & name)
			{
				static_assert(Dimension == 1);
				Names = {name};
				bufferWidth = std::max(bufferWidth,(int)name.length());
			}

			//! Deletes all levels of the bar from the screen, useful once the bar has finished and is no longer needed
			void Clear()
			{
				ClearScreen();
			}
	};
}