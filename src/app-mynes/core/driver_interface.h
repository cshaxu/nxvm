#ifndef CORE_DRIVER_INTERFACE_H
#define CORE_DRIVER_INTERFACE_H

#include "common/machine/machine_interface.h"

typedef struct core_driver core_driver;

typedef struct core_driver_options {
    lib_u8 initial_ram_byte;
    lib_bool text_output;
} core_driver_options;

lib_status core_driver_create(core_driver **out_driver,
    const core_driver_options *options);
lib_status core_driver_make_driver(core_driver *driver,
    common_machine_driver *out_common_driver);
lib_status core_driver_destroy(core_driver *driver);
void core_driver_request_input_reset(core_driver *driver);
lib_bool core_driver_has_cartridge(const core_driver *driver);
lib_status core_driver_load_battery_ram(core_driver *driver, const char *path);
lib_status core_driver_save_battery_ram(core_driver *driver, const char *path);

#endif
