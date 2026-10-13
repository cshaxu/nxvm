#ifndef MACHINE_DRIVER_INTERFACE_H
#define MACHINE_DRIVER_INTERFACE_H

#include "emulator/machine/machine_interface.h"

typedef struct machine_driver machine_driver;

typedef struct machine_driver_options {
    lib_u8 initial_ram_byte;
    lib_bool text_output;
    lib_bool audio_enabled;
} machine_driver_options;

lib_status machine_driver_create(machine_driver **out_driver,
    const machine_driver_options *options);
lib_status machine_driver_make_driver(machine_driver *driver,
    emulator_machine_driver *out_emulator_driver);
lib_status machine_driver_destroy(machine_driver *driver);
void machine_driver_request_input_reset(machine_driver *driver);
lib_bool machine_driver_has_cartridge(const machine_driver *driver);
lib_status machine_driver_load_battery_ram(machine_driver *driver, const char *path);
/* Call only at a quiescent boundary or after executor shutdown. No cartridge,
 * no battery or clean RAM is successful no-work. Failure retains dirty RAM. */
lib_status machine_driver_save_battery_ram(machine_driver *driver, const char *path);

#endif
