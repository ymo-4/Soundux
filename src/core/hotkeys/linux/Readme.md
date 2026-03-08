`event-codes-parse.py`: produce `event-codes-is.h` and `event-codes-si.h` from `input-event-codes.h` using `python event-codes-parse.py`

`input-event-codes.h`: trimmed (only keyboard keys) version of `/usr/src/linux/include/uapi/linux/input-event-codes.h`

`event-codes-is.h`: parsed key codes hashmap int:string

`event-codes-si.h`: parsed key codes hashmap string:int

`hotkeys-adaptor.h`: generated from `org.soundux.hotkeys.xml` using `sdbus-c++-xml2cpp --adaptor="hotkeys-adaptor.h" org.soundux.hotkeys.xml`
