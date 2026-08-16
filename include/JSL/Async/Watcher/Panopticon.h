#pragma once
#include "FileWatcher.h"
#include "InputWatcher.h"
#include "SocketWatcher.h"
#include <atomic>
#include <condition_variable>
#include <deque>
#include <map>
#include <optional>
#include <variant>
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
		std::variant<std::string, std::set<FileChange>> Data;
	};

	class Panopticon
	{
		using strCallBack = std::function<void(std::string)>;
		using batchCallBack = std::function<void(std::set<FileChange>)>;
		using fileCallBack = std::function<void(FileChange)>;

	  public:
		Panopticon();
		void SetInputCallback(strCallBack fcn, std::optional<std::string> exitString = std::nullopt, bool overwriteExisting = false);
		void SetSocketCallback(std::string socketID, strCallBack fcn, bool forceAcquire = false, bool overwriteExisting = false);
		void SetFileBatchCallback(std::string watchedDirectory, bool recursive, batchCallBack fcn, bool overwriteExisting = false);
		void SetSingleFileCallback(std::string watchedDirectory, bool recursive, fileCallBack fcn, bool overwriteExisting = false);
		void Start();
		void Stop();

	  private:
		void Shutdown();
		std::mutex Queue;
		std::condition_variable AwaitingInstruction;
		Watcher::Input InputTracker{};
		std::deque<Instruction> Instructions = {};
		std::atomic<bool> IsRunning{false};

		strCallBack cinCallback;
		std::map<std::string, std::unique_ptr<Watcher::Socket>> SocketTracker{};
		std::map<std::string, strCallBack> socketCallback;
		std::map<std::string, std::unique_ptr<Watcher::File>> FileTracker{};
		std::map<std::string, batchCallBack> fileCallback;
	};
}; // namespace JSL::Async::Watcher
