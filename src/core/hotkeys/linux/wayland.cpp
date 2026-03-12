#if defined(__linux__)
#include <core/global/globals.hpp>
#include <unistd.h>
#include "../hotkeys.hpp"
#include <fancy.hpp>
#include <sdbus-c++/sdbus-c++.h>
#include <thread>
#include "hotkeys-adaptor.h"
#include <linux/uinput.h>
#include <fcntl.h>
#include "event-codes-is.h"
#include "event-codes-si.h"

using namespace std::chrono_literals;

class HotkeysAdaptor : public sdbus::AdaptorInterfaces<org::soundux::hotkeys_adaptor>
{
public:
    HotkeysAdaptor(Soundux::Objects::Hotkeys *hk, sdbus::IConnection& conn, sdbus::ObjectPath obj)
        : AdaptorInterfaces(conn, std::move(obj))
        , hotkeys(hk)
    {
        registerAdaptor();
    }

    ~HotkeysAdaptor() {
        unregisterAdaptor();
    }

    Soundux::Objects::Hotkeys *hotkeys;

protected:
    void emit(const std::vector<int32_t> &keys) override {
        for (int i = 0; i < keys.size(); i++) {
            hotkeys->onKeyDown(keys[i]);
        }

        for (int i = 0; i < keys.size(); i++) {
            hotkeys->onKeyUp(keys[i]);
        }
    }

    void emit_name(const std::vector<std::string> &keys) override {
        size_t sz = keys.size();
        int* i_keys = reinterpret_cast<int*>(malloc(sz * sizeof(int)));

        for (int i = 0; i < sz; i++) {
            auto it = event_codes_si.find(keys[i]);
            if (it == event_codes_si.end()) {
                std::cerr << "[dbus: emit_name] Cannot find '" << keys[i] << "'" << '\n';
                return;
            }

            i_keys[i] = it->second;
        }


        for (int i = 0; i < sz; i++) {
            hotkeys->onKeyDown(i_keys[i]);
        }

        for (int i = 0; i < sz; i++) {
            hotkeys->onKeyUp(i_keys[i]);
        }

        free(i_keys);
    }
};

void uinput_emit(int fd, int type, int code, int val) {
    struct input_event ie;
    ie.type = type;
    ie.code = code;
    ie.value = val;
    /* timestamp values below are ignored */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write(fd, &ie, sizeof(ie));
}

namespace Soundux::Objects
{
    void Hotkeys::listen()
    {
        if (!Soundux::Globals::disable_uinput) 
        {
            fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
            if (fd <= 0) {
                std::cerr << "Failed to open /dev/uinput (Do you have root/permission?)" << '\n';
            } else {
                // Enable key events and the specific key (e.g., the 'A' key)
                ioctl(fd, UI_SET_EVBIT, EV_KEY);
                // register every possible key so we can emit it later
                for (int i = 0; i <= KEY_MAX; ++i) {
                    ioctl(fd, UI_SET_KEYBIT, i);
                }

                struct uinput_setup usetup;
                memset(&usetup, 0, sizeof(usetup));
                usetup.id.bustype = BUS_USB;
                usetup.id.vendor = 0x1234; /* sample vendor */
                usetup.id.product = 0x5678; /* sample product */
                strcpy(usetup.name, "Soundux virtual keyboard");

                ioctl(fd, UI_DEV_SETUP, &usetup);
                ioctl(fd, UI_DEV_CREATE);
            }
        } else {
            std::cout << "Not initializing uinput as args says" << '\n';
        }

        // Create D-Bus connection to (either the system or session) bus and request a well-known name on it.
        sdbus::ServiceName serviceName{"org.soundux.hotkeys"};
        dbus_conn = sdbus::createBusConnection(serviceName);

        // Create concatenator D-Bus object.
        sdbus::ObjectPath objectPath{"/org/soundux/hotkeys"};
        HotkeysAdaptor hotkeys(this, *dbus_conn, std::move(objectPath));

        dbus_conn->enterEventLoop();
    }

    std::string Hotkeys::getKeyName(const int &key)
    {
        auto it = event_codes_is.find(key);
        if (it != event_codes_is.end()) {
            return it->second;
        }

        std::cerr << "[getKeyName] Couldn't find '" << key << "'" << '\n';
        return "KEY_" + std::to_string(key);
    }

    void Hotkeys::stop()
    {
        dbus_conn->leaveEventLoop();
        listener.join();
        if (fd > 0) {
            ioctl(fd, UI_DEV_DESTROY);
            close(fd);
        }
    }

    void Hotkeys::pressKeys(const std::vector<int> &keys)
    {
        if (fd <= 0) {
            std::cerr << "uinput not initialized, pressKeys disabled" << '\n';
            return;
        }

        keysToPress = keys;
        for (const auto &key : keys)
        {
//            XTestFakeKeyEvent(display, key, True, 0);
            uinput_emit(fd, EV_KEY, key, 1);
        }

        uinput_emit(fd, EV_SYN, SYN_REPORT, 0);
    }

    void Hotkeys::releaseKeys(const std::vector<int> &keys)
    {
        if (fd <= 0) {
            std::cerr << "uinput not initialized, releaseKeys disabled" << '\n';
            return;
        }

        keysToPress.clear();
        for (const auto &key : keys)
        {
//            XTestFakeKeyEvent(display, key, False, 0);
            uinput_emit(fd, EV_KEY, key, 0);
        }

        uinput_emit(fd, EV_SYN, SYN_REPORT, 0);
    }
} // namespace Soundux::Objects


#endif
