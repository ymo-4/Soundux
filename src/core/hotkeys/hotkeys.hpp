#pragma once
#include <atomic>
#include <sdbus-c++/sdbus-c++.h>
#include <string>
#include <thread>
#include <vector>

namespace Soundux
{
    namespace Objects
    {
        class Hotkeys
        {
            std::thread listener;
            std::unique_ptr<sdbus::IConnection> dbus_conn;
            int fd = 0;
            std::atomic<bool> kill = false;
            std::atomic<bool> notify = false;

            std::vector<int> pressedKeys;
            std::vector<int> keysToPress;
#if defined(_WIN32)
            std::thread keyPressThread;
            std::atomic<bool> shouldPressKeys = false;
#endif

          private:
            void listen();

          public:
            void init();
            void stop();
            void shouldNotify(bool);

            void onKeyUp(int);
            void onKeyDown(int);

            void pressKeys(const std::vector<int> &);
            void releaseKeys(const std::vector<int> &);

            std::string getKeyName(const int &);
            std::string getKeySequence(const std::vector<int> &);
        };
    } // namespace Objects
} // namespace Soundux
