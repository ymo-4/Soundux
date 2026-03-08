# Soundux with dbus hotkeys + uinput push to talk support
See `src/core/hotkeys/linux/` directory for more info

Added features:
* uinput (virtual keyboard) push to talk
* `--disable-uinput` to disable push to talk
* dbus hotkeys (see [dbus](#dbus))

## dbus
Use your compositor settings (hyprland, kwin, etc) to run:
```
dbus-send --print-reply \
    --dest="org.soundux.hotkeys" \
    "/org/soundux/hotkeys" \
    "org.soundux.hotkeys.emit_name" \
    "array:string:KEY_5"
```
* `--print-reply` is essential, without it dbus-send sends a signal instead of calling a method
* you can use key combinations like `array:string:KEY_LEFTSHIFT,KEY_X` means `KEY_LEFTSHIFT` + `X`
but it is basically useless everywhere but for push to talk
* you can change `emit_name` to `emit`, `array:string:...` to `array:int32:6` so you can use not
only key names but key ids too, useless tbh
* change `KEY_5` by your key. You can see all keys in `src/core/hotkeys/linux/input-event-codes.h`

# Notes
To build:
1. `cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5`
2. `cd build`
3. `cmake --build . -j`

Requires `sdbus-c++` installed!
