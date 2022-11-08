#pragma once
#include "ProgressBar.h"
#include "../Strings/Time.h"
#include <chrono>
namespace JSL
{
	
	template<int Dimension = 1, char Symbol = '#',unsigned int MaxHashes =20>
	class PredictionBar : public ProgressBar<Dimension,true,Symbol,MaxHashes>
	{
		public:

			template<typename... Ts>
			PredictionBar(Ts... targetCounts) : ProgressBar<Dimension,true,Symbol,MaxHashes>(targetCounts...)
			{
				Start = std::chrono::system_clock::now();
				UpdateTime = Start;
			}


			template<typename... Ts>
			void Update(Ts... positions)
			{
				static_assert(sizeof...(positions) == Dimension);
				this->reprintNeeded  = false;
				this->UnrollPositions(0,positions...);
				ComputeTime();
				//only reprint the bar if something has changed
				if (this->reprintNeeded)
				{
					PrintPositions_DeleteMode();

				}
			}

		private:
			std::chrono::time_point<std::chrono::system_clock> Start;
			std::chrono::time_point<std::chrono::system_clock> UpdateTime;
			double CurrentDuration;
			double Prediction = 0;
			double prevProgress = 0;
			void ComputeTime()
			{
				std::chrono::time_point<std::chrono::system_clock> Now = std::chrono::system_clock::now();
				std::chrono::duration<double,std::ratio<1,1>> totalDuration = Now - Start;
				std::chrono::duration<double,std::ratio<1,1>> updateDuration = Now - UpdateTime;
				CurrentDuration = totalDuration.count();
				if (updateDuration.count() > 1)
				{
					this->reprintNeeded = true;
				}
				if (this->reprintNeeded)
				{
					UpdateTime = Now; // also catches reprints due to altered barstates!
				
				
					double totalProg = 0;
					double next = 1;
					for (int i = 0; i < Dimension; ++i)
					{
						double prog = this->BarProgress[i] * (this->BarTargets[i]-1)/(this->BarTargets[i]);
						totalProg += prog * next;
						next = next/(this->BarTargets[i]);
						// std::cout << i << "  " << prog << "  " << next << "  " << totalProg << std::endl;
					}
					
					double globalRate = totalProg/totalDuration.count();
					double stepRate = (totalProg - prevProgress)/updateDuration.count();
					double w = 0.5;
					double meanRate = (1.0 - w)*globalRate + w*stepRate;
					double anticipate = std::max(0.0,Prediction - updateDuration.count());
					double mem = 0.99;
					double activePrediction = (1.0 - totalProg)/meanRate;
					if (activePrediction > anticipate)
					{
						mem = 0.5;
					}
					
					
					// std::cout << "Anticiapting " << Prediction << " - " << updateDuration.count() << " = " << Prediction - updateDuration.count() << " = " << anticipate << " actively " << activePrediction << " av = " << mem * anticipate + (1.0 - mem)*activePrediction<< "\n\n\n\n";
					Prediction = mem * anticipate + (1.0 - mem)*activePrediction;
					// std::cout << "Estimated " << totalProg << " through at rates " << globalRate << "   " << stepRate << "\n\n\n\n";
					prevProgress = totalProg;
				}
			}
			virtual void PrintPositions_DeleteMode()
			{

				if (!this->firstPrint)
				{
					this->ClearScreen();
				}
				this->firstPrint = false;
				for (int i = 0; i < Dimension; ++i)
				{
					int newHashes = this->Hashes[i];
					if (this->Names.size() > 0)
					{
						std::cout << std::setw(this->bufferWidth)  << this->Names[i]<< " ";
					}
					std::cout << "[" << std::string(newHashes,Symbol) << std::string(MaxHashes - newHashes,' ') << "]";
					if (i == Dimension - 1)
					{
						std::cout << " ETR: " << FormatDuration(Prediction);
					}
					if (i == 0)
					{
						std::cout << " " << (int)(100*prevProgress) << "%" << " in " << JSL::FormatDuration(CurrentDuration);
					}
					std::cout << "\n";
				}
			}
	};


}