#pragma once
#include "ManagerBase.h"
#include <JSL/Async/ParallelPool.h>
// #include <JSL.h>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string_view>
namespace JSL::Event::Serial
{

	class Manager : public internal::HandlerBase
	{
	  public:
		Manager(std::string_view identifier);
		Manager(Watcher &watcher);

		void AddTask(Task::Instruction job);

		void Synchroniser() {}; // does nothing!
	};
} // namespace JSL::Event::Serial

namespace JSL::Event::Async
{
	class Manager : public internal::HandlerBase
	{
	  public:
		Manager(size_t ncores, std::string_view identifier);
		Manager(size_t ncores, Watcher &watcher);

		void SerialTask(Task::Instruction job); // adds jobs into the Dispatcher queue, which execute in serial, whilst no other threads active

		void AddTask(Task::Instruction job);

		void Synchroniser();

	  private:
		JSL::Parallel::Pool Workers;
	};

} // namespace JSL::Event::Async
