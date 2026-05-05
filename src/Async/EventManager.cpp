#include <JSL.h>

namespace JSL
{
	SerialEventManager::SerialEventManager(std::string_view id) : HandlerBase(id)
	{

	}

	
	
	void SerialEventManager::AddTask(Task::Instruction job)
	{
		std::lock_guard lock(Sync);
		TaskQueue.push(std::move(job));
		CV.notify_one();
		//lock released when guard goes out of scope
	}

	ParallelEventManager::ParallelEventManager(size_t ncores, std::string_view id) : internal::HandlerBase(id), Workers(ncores)
	{

	}


	void ParallelEventManager::AddTask(Task::Instruction job)
	{
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
			Workers.AsyncTask([this,job](){
				this->Process(job);
			});
		}
	}

	void ParallelEventManager::SerialTask(Task::Instruction job)
	{
		std::lock_guard lock(Sync);
		TaskQueue.push(std::move(job));
		CV.notify_one();
	}

	void ParallelEventManager::Synchroniser()
	{
		Workers.Synchronise();
	}
}