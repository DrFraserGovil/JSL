#pragma once
#include <JSL/Async/Watcher.h>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string_view>
#include <functional>
namespace JSL
{
	namespace Task
	{
		enum Origin
		{
			CIN,
			FILE,
			SOCKET,
			SHUTDOWN,
		};
		enum Priority
		{
			NORMAL,
			LOCKING
		};
		struct Instruction
		{
			Origin origin;
			Priority priority;
			std::variant<std::string, JSL::FileChange> payload;
			Instruction(Origin in, std::string_view msg, Priority order = Priority::NORMAL)
			{
				origin = in;
				priority = order;
				payload = static_cast<std::string>(msg);
			}
			Instruction(Origin in, FileChange msg, Priority order = Priority::NORMAL)
			{
				origin = in;
				priority = order;
				payload = msg;
			}
			static Instruction Shutdown()
			{
				return Instruction(Origin::SHUTDOWN,"",Priority::LOCKING);
			}
		};
	}


	namespace internal
	{
		class HandlerBase
		{
			public:

				void SetCinCallback(std::function<void(std::string_view)> callback);
				void SetSocketCallback(std::function<void(std::string_view)> callback);
				void SetTextCallback(std::function<void(std::string_view)> callback);
				void SetFileCallback(std::function<void(JSL::FileChange)> callback);

				std::set<std::string,std::less<>> ShutdownCommands;

				virtual void AddTask(Task::Instruction job) = 0;
				void Run();

				// Delete copy and move to be explicit
				HandlerBase(const HandlerBase&) = delete;
				HandlerBase& operator=(const HandlerBase&) = delete;
				HandlerBase(HandlerBase&&) = delete;
				HandlerBase& operator=(HandlerBase&&) = delete;
			protected:
				HandlerBase(std::string_view id);
				void Process(Task::Instruction job);
				void Exit();
				void InitialiseRun();

				std::function<void(std::string_view)> callback_cin;
				std::function<void(std::string_view)> callback_socket;
				std::function<void(JSL::FileChange)> callback_file;

				Watcher Watch;
				virtual void Synchroniser() = 0;
				std::mutex Sync;
				std::condition_variable CV;
				std::queue<Task::Instruction> TaskQueue;
		};
	}
}