#pragma once
#include <JSL/Async/Socket.h>
#include <string>
#include <poll.h>
#include <functional>
#include <filesystem>
#include <string_view>
#include <map>
#include <chrono>
#include <sys/inotify.h>
#include <queue>
#include <set>
#include <atomic>
namespace JSL::Async
{

    struct FileChange
    {
        std::filesystem::path Path;
        uint32_t Mask; 
        std::strong_ordering operator<=>(const FileChange& other) const
        {
            return Path <=>other.Path;
        };
    };

    class Watcher
    {
        public:
            //only way to create the object is through a factory function that ensures all the relevant objectsd exist (and throws if not)
            static std::optional<Watcher> Create(std::string_view socketName,double timeout=2, bool forceAcquire=false);

            void Run();
            void SetSocketTimeout(double seconds);
            void SetMaxRuntime(double minutes);
            void SetBlockingTime(double seconds);
            void SetDebounce(size_t milliseconds);

            void SetSocketCallback(std::function<void(std::string_view)> callback);

            void SetInotifyCallback(std::function<void(const FileChange &)> callback);
            
            void SetCInCallback(std::function<void(std::string_view)> callback);

            int Watch(std::filesystem::path path,uint32_t watchFlags = IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVE | IN_IGNORED);
            void Unwatch(int id);
            void Shutdown();
            void Unwatch(std::filesystem::path path);
            void Message(std::string_view msg);
            std::set<std::filesystem::path> GetWatchedFiles();
            std::chrono::steady_clock::duration GetRuntime();
            bool Running(){return *LoopRunning;};
            void PrepRun();
        private:
            std::unique_ptr<std::atomic<bool>> LoopRunning = std::make_unique<std::atomic<bool>>(false);
            bool BlockNewAdds = false;
            Watcher(Antenna && socketIn);
            Antenna Receiver;
            Antenna::Hotline Sender;
            std::vector<pollfd> PollCatchers;
            std::map<int,std::function<void()>> Callbacks;
            std::chrono::steady_clock::duration Timeout; // maximum runtime before exit
            int BlockingTime = 50; //milliseconds
            int INotifyID = -1;
            void InitialiseINotify();
            std::map<int, std::filesystem::path> WatchMap;
            void UpdateFileBatch();
            size_t DebounceMS = 50;
            std::optional<std::chrono::steady_clock::time_point> InitialiseTime;

            //special treatment for the inotify debounce
            bool AwaitingDebounce;
            std::chrono::steady_clock::time_point Wakeup;
            std::function<void(FileChange)> CachedCallback; //pass by copy so can do async without needing a mutex
            std::set<FileChange> FileCache = {};



    };
};
