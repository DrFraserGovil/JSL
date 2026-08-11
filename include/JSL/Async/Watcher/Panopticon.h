#pragma once
#include "InputWatcher.h"
#include <atomic>
#include <condition_variable>
#include <deque>
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
		void Start();
		void Stop();

	  private:
		void Shutdown();
		std::mutex Queue;
		std::condition_variable AwaitingInstruction;
		Watcher::Input Input{};
		std::deque<Instruction> Instructions = {};
		callback cinCallback;
		std::atomic<bool> IsRunning;
	};
}; // namespace JSL::Async::Watcher
