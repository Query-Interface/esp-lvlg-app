crash in UI

Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.

Core  1 register dump:
PC      : 0x4200d561  PS      : 0x00060030  A0      : 0x8200d5a1  A1      : 0x3fced2d0  
--- 0x4200d561: std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_data() const at /Users/I051413/.espressif/tools/xtensa-esp-elf/esp-14.2.0_20260121/xtensa-esp-elf/xtensa-esp-elf/include/c++/14.2.0/bits/basic_string.h:228
--- (inlined by) std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) at /Users/I051413/.espressif/tools/xtensa-esp-elf/esp-14.2.0_20260121/xtensa-esp-elf/xtensa-esp-elf/include/c++/14.2.0/bits/basic_string.h:556
A2      : 0x3fced330  A3      : 0x0000003c  A4      : 0x3fcaa880  A5      : 0x3fcaa0e8  
A6      : 0x3fcaca8c  A7      : 0x0000002e  A8      : 0x3fced338  A9      : 0x3fced2b0  
A10     : 0x3fced330  A11     : 0x3fced4b0  A12     : 0x00000000  A13     : 0x3fced470  
A14     : 0x00000003  A15     : 0x00000000  SAR     : 0x00000005  EXCCAUSE: 0x0000001c  
EXCVADDR: 0x0000003c  LBEG    : 0x400555cd  LEND    : 0x400555e1  LCOUNT  : 0xfffffffd  
--- 0x400555cd: strcpy in ROM
--- 0x400555e1: strcpy in ROM


Backtrace: 0x4200d55e:0x3fced2d0 0x4200d59e:0x3fced2f0 0x42026d95:0x3fced370 0x42026e42:0x3fced390 0x42034ed7:0x3fced3d0 0x42034eb2:0x3fced3f0 0x42034fc2:0x3fced410 0x4200dd74:0x3fced440 0x42034eb2:0x3fced490 0x42034fc2:0x3fced4b0 0x4200d281:0x3fced4e0 0x42026d95:0x3fced500 0x42026e42:0x3fced520 0x42045212:0x3fced560 0x42123e41:0x3fced5a0 0x42026d62:0x3fced5c0 0x42026e42:0x3fced5e0 0x42027d43:0x3fced620 0x42027f3f:0x3fced640 0x42028051:0x3fced660 0x42041ec5:0x3fced6a0 0x42041f72:0x3fced6c0 0x4200abf8:0x3fced6e0 0x40384b7d:0x3fced700
--- 0x4200d55e: std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_local_data() at /Users/I051413/.espressif/tools/xtensa-esp-elf/esp-14.2.0_20260121/xtensa-esp-elf/xtensa-esp-elf/include/c++/14.2.0/bits/basic_string.h:235
--- (inlined by) std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) at /Users/I051413/.espressif/tools/xtensa-esp-elf/esp-14.2.0_20260121/xtensa-esp-elf/xtensa-esp-elf/include/c++/14.2.0/bits/basic_string.h:553
--- 0x4200d59e: weather_event_cb(_lv_event_t*) at /Users/I051413/pProject/ESP32-TUX/main/gui.hpp:1002
--- 0x42026d95: event_send_core at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/core/lv_event.c:461
--- 0x42026e42: lv_event_send at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/core/lv_event.c:74
--- 0x42034ed7: obj_notify_cb at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/extra/others/msg/lv_msg.c:170
--- 0x42034eb2: notify at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/extra/others/msg/lv_msg.c:162
--- 0x42034fc2: lv_msg_send at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/extra/others/msg/lv_msg.c:122
--- 0x4200dd74: tux_ui_change_cb(void*, lv_msg_t*) at /Users/I051413/pProject/ESP32-TUX/main/main.cpp:388
--- 0x42034eb2: notify at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/extra/others/msg/lv_msg.c:162
--- 0x42034fc2: lv_msg_send at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/extra/others/msg/lv_msg.c:122
--- 0x4200d281: footer_button_event_handler(_lv_event_t*) at /Users/I051413/pProject/ESP32-TUX/main/gui.hpp:1028
--- 0x42026d95: event_send_core at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/core/lv_event.c:461
--- 0x42026e42: lv_event_send at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/core/lv_event.c:74
--- 0x42045212: lv_btnmatrix_event at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/widgets/lv_btnmatrix.c:462
--- 0x42123e41: lv_obj_event_base at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/core/lv_event.c:96
--- 0x42026d62: event_send_core at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/core/lv_event.c:452
--- 0x42026e42: lv_event_send at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/core/lv_event.c:74
--- 0x42027d43: indev_proc_press at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/core/lv_indev.c:886
--- 0x42027f3f: indev_pointer_proc at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/core/lv_indev.c:380
--- 0x42028051: lv_indev_read_timer_cb at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/core/lv_indev.c:101
--- 0x42041ec5: lv_timer_exec at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/misc/lv_timer.c:313
--- 0x42041f72: lv_timer_handler at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/misc/lv_timer.c:109
--- 0x4200abf8: gui_task(void*) at /Users/I051413/pProject/ESP32-TUX/managed_components/lvgl__lvgl/src/lv_api_map.h:37
--- (inlined by) gui_task at /Users/I051413/pProject/ESP32-TUX/main/helpers/helper_display.hpp:210
--- 0x40384b7d: vPortTaskWrapper at /Users/I051413/.espressif/v5.5.4/esp-idf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:139




ELF file SHA256: eee09421e

Rebooting...
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0xc (RTC_SW_CPU_RST),boot:0x2b (SPI_FAST_FLASH_BOOT)
Saved PC:0x4038097a
--- 0x4038097a: esp_cpu_wait_for_intr at /Users/I051413/.espressif/v5.5.4/esp-idf/components/esp_hw_support/cpu.c:64
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fce2820,len:0x14f0
load:0x403c8700,len:0xd24
load:0x403cb700,len:0x2ef0
entry 0x403c8924