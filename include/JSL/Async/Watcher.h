#pragma once
#include <JSL/Async/Socket.h>
#include <string>
#include <functional>
#include <filesystem>
#include <string_view>
#include <map>
#include <chrono>
#include <queue>
#include <set>
#include <atomic>

#if defined(_WIN32) || defined(_WIN64)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h> // Provides WSAPollfd
    using pollfd_t = WSAPOLLFD;
    using watch_id_t = void*; // Maps to HANDLE in implementation

    // Abstract the Linux masks so your API footprint doesn't change
    #define IN_MODIFY  0x00000002
    #define IN_CREATE  0x00000100
    #define IN_DELETE  0x00000200
    #define IN_MOVE    0x000000c0
    #define IN_IGNORED 0x00008000
#else
    #include <poll.h>
    #include <sys/inotify.h>
    using pollfd_t = pollfd;
    using watch_id_t = int;
#endif
namespace JSL::Event
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
            watch_id_t INotifyID = reinterpret_cast<watch_id_t>(-1);
            void InitialiseINotify();
            std::map<int, std::filesystem::path> WatchMap;
            #if defined(_WIN32) || defined(_WIN64)
                // Stores the unique directory HANDLEs required by Windows
                std::map<int, void*> WinDirectoryHandles;
            #endif
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
