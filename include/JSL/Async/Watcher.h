#pragma once
#include <JSL/Async.h>
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
namespace JSL
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
            static std::optional<Watcher> Create(const std::string & socketName,double timeout=2, bool forceAcquire=false);

            void Run();
            void SetSocketTimeout(double seconds);
            void SetMaxRuntime(double minutes);
            void SetBlockingTime(double seconds);


            void SetSocketCallback(std::function<void(std::string_view)> callback);

            void SetInotifyCallback(std::function<void(const FileChange &)> callback);
            
            void SetCInCallback(std::function<void(std::string_view)> callback);

            int Watch(std::filesystem::path path,uint32_t watchFlags = IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVE);
            void Unwatch(int id);
            void Unwatch(std::filesystem::path path);
            void Message(const std::string & msg);
        private:
            bool BlockNewAdds = false;
            Watcher(Socket && socketIn);
            Socket Receiver;
            std::optional<Socket> Sender;
            std::vector<pollfd> PollCatchers;
            std::map<int,std::function<void()>> Callbacks;

            std::chrono::steady_clock::duration Timeout; // maximum runtime before exit
            int BlockingTime = 50; //milliseconds
            int INotifyID = -1;
            void InitialiseINotify();
            std::map<int, std::filesystem::path> WatchMap;
            std::set<FileChange> GetFileBatch();
            size_t DebounceMS = 50;

            std::set<int> CurrentDebounce;

            struct debouncer
            {
                int ID;
                std::chrono::steady_clock::time_point ActivationTime;
                std::function<void()> Function;
                debouncer(int id, std::chrono::steady_clock::time_point time,  std::function<void()> func): ID(id), ActivationTime(time), Function(func)
                {};
            };

            std::queue<debouncer> DebounceQueue;


    };
};
