#pragma once
#include "../Watcher.h"
#include <atomic>
#include <condition_variable>
#include <deque>
#include <map>
#include <optional>
namespace JSL::Async::Watcher
{
	// The All-Seer

	struct Instruction
	{
		enum class Type
		{
			CIN,
			FILE,
			SOCKET,
			SHUTDOWN
		};

		Type Origin;
		std::string ID;
		std::string Message;
	};

	class Panopticon
	{
		using callback = std::function<void(std::string)>;

	  public:
		Panopticon();
		void SetInputCallback(callback fcn, std::optional<std::string> exitString = std::nullopt);
		void SetSocketCallback(std::string socketID, callback fcn, bool forceAcquire = false);
		void Start();
		void Stop();

	  private:
		void Shutdown();
		std::mutex Queue;
		std::condition_variable AwaitingInstruction;
		Watcher::Input Input{};
		std::deque<Instruction> Instructions = {};
		std::atomic<bool> IsRunning;

		callback cinCallback;
		std::map<std::string, std::unique_ptr<Watcher::Socket>> Socket{};
		std::map<std::string, callback> socketCallback;
	};
}; // namespace JSL::Async::Watcher
