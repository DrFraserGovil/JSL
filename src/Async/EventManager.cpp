#include <JSL/Async/EventManager.h>

namespace JSL::Event::Serial
{
	Manager::Manager(std::string_view id) : HandlerBase(id)
	{
	}
	Manager::Manager(Watcher &watch) : HandlerBase(watch)
	{
	}

	void Manager::AddTask(Task::Instruction job)
	{
		ProtectAdd();
		std::lock_guard lock(Sync);
		TaskQueue.push(std::move(job));
		CV.notify_one();
		// lock released when guard goes out of scope
	}
} // namespace JSL::Event::Serial

namespace JSL::Event::Async
{
	Manager::Manager(size_t ncores, std::string_view id) : internal::HandlerBase(id), Workers(ncores)
	{
	}

	Manager::Manager(size_t ncores, Watcher &watch) : internal::HandlerBase(watch), Workers(ncores)
	{
	}

	void Manager::AddTask(Task::Instruction job)
	{
		ProtectAdd();
		bool serialQueued;
		{
			std::lock_guard lock(Sync);
			serialQueued = !(TaskQueue.empty());
		}
		if (job.priority == Task::LOCKING || serialQueued)
		{
			SerialTask(job);
		}
		else
		{
			Workers.AsyncTask([this, job]()
				{ this->Process(job); });
		}
	}

	void Manager::SerialTask(Task::Instruction job)
	{
		std::lock_guard lock(Sync);
		TaskQueue.push(std::move(job));
		CV.notify_one();
	}

	void Manager::Synchroniser()
	{
		Workers.Synchronise();
	}

} // namespace JSL::Event::Async
