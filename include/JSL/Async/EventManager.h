#pragma once
#include <JSL.h>
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
		struct Instruction
		{
			Origin origin;
			std::variant<std::string, JSL::FileChange> payload;
			Instruction(Origin in, std::string_view msg)
			{
				origin = in;
				payload = static_cast<std::string>(msg);
			}
			Instruction(Origin in, FileChange msg)
			{
				origin = in;
				payload = msg;
			}
			static Instruction Shutdown()
			{
				return Instruction(Origin::SHUTDOWN,"");
			}
		};
	}

	class EventManager
	{
		public:
			static std::unique_ptr<EventManager> Create(std::string_view identifier);	
			EventManager(Watcher && w);
			
			void Run();

			void SetCinCallback(std::function<void(std::string_view)> callback);
			void SetSocketCallback(std::function<void(std::string_view)> callback);
			void SetTextCallback(std::function<void(std::string_view)> callback);
			void SetFileCallback(std::function<void(JSL::FileChange)> callback);
			void AddTask(Task::Instruction job);

			std::set<std::string,std::less<>> ShutdownCommands;

			// Delete copy and move to be explicit
			EventManager(const EventManager&) = delete;
			EventManager& operator=(const EventManager&) = delete;
			EventManager(EventManager&&) = delete;
			EventManager& operator=(EventManager&&) = delete;
		private:
			
			void Exit();

			std::queue<Task::Instruction> TaskQueue;

			Watcher Watch;

			std::function<void(std::string_view)> callback_cin;
			std::function<void(std::string_view)> callback_socket;
			std::function<void(JSL::FileChange)> callback_file;
			
			void Process(Task::Instruction job);
			//these are pointers to make the class movable
			std::mutex Sync;
			std::condition_variable CV;

			
	};
}