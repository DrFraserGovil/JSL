#pragma once

#include <string>
#include <vector>
#include "clearScreen.h"
namespace JSL
{
	template<int Dimension = 1, bool DeleteMode = true, char Symbol = '#'>
	class ProgressBar
	{
		private:
			int MaxHashes = 32;
			bool firstPrint = true;
			std::vector<int> BarTargets;
			std::vector<double> BarProgress;
			std::vector<int> Hashes;
			std::vector<std::string> Names;
			const int BarCount;
			bool reprintNeeded;
			int bufferWidth = 10;
			int prevHashes = 0;

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
			void ClearScreen()
			{
				for (int i = 0; i < BarCount; ++i)
				{
					jumpLineUp();
					deleteLine();
				}
			}
			void PrintPositions_DeleteMode()
			{
				if (!firstPrint)
				{
					ClearScreen();
				}
				firstPrint = false;
				for (int i = 0; i < BarCount; ++i)
				{
					int newHashes = Hashes[i];
					if (Names.size() > 0)
					{
						std::cout << std::setw(bufferWidth)  << Names[i]<< " ";
					}
					std::cout << "[" << std::string(newHashes,Symbol) << std::string(MaxHashes - newHashes,' ') << "]\n";
				}
			}
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
			template<typename... Ts>
			ProgressBar(Ts... target) : BarCount(sizeof...(target))
			{
				static_assert(sizeof...(target) == Dimension);
				UnpackTargets(target...);
			}

			template<typename... Ts>
			void Update(Ts... positions)
			{
				static_assert(sizeof...(positions) == Dimension);
				reprintNeeded  = false;
				UnrollPositions(0,positions...);

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
			void SetName(const std::string & name)
			{
				static_assert(Dimension == 1);
				Names = {name};
				bufferWidth = std::max(bufferWidth,(int)name.length());
			}
			void SetName(const std::vector<std::string> & names)
			{
				Assert("Name vector must be same size as bar count",names.size() == Dimension);
				Names = names;
				for (int i = 0; i < Dimension; ++i)
				{
					bufferWidth = std::max(bufferWidth,(int)names[i].length());
				}
			}
			void SetName(unsigned int idx, const std::string & name)
			{
				Assert("Assign name to bar between 0 and BarCount-1", idx < BarCount);
				if (Names.size() < BarCount)
				{
					Names = std::vector<std::string>(BarCount,"");
				}
				Names[idx] = name;
				bufferWidth = std::max(bufferWidth,(int)name.length());
			}


			void Clear()
			{
				ClearScreen();
			}
	};
}