#pragma once
#include "ManagerBase.h"
#include <JSL/Async/ParallelPool.h>
// #include <JSL.h>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string_view>
#include <functional>
namespace JSL::Async
{
	

	class SerialEventManager: public internal::HandlerBase
	{
		public:
			SerialEventManager(std::string_view identifier);
			SerialEventManager(Watcher & watcher);
			
			void AddTask(Task::Instruction job);
			
			void Synchroniser(){}; //does nothing!
	};


	class ParallelEventManager : public internal::HandlerBase
	{
		public:
			ParallelEventManager(size_t ncores, std::string_view identifier);
			ParallelEventManager(size_t ncores, Watcher & watcher);

			void SerialTask(Task::Instruction job); //adds jobs into the Dispatcher queue, which execute in serial, whilst no other threads active

			void AddTask(Task::Instruction job);

			void Synchroniser();
		private:
			Pool Workers;
	};

}