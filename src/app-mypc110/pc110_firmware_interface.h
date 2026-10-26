#ifndef MYPC110_FIRMWARE_INTERFACE_H
#define MYPC110_FIRMWARE_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-core/chips/vl82c420/vl82c420_interface.h"
#include "x86-core/machine/firmware_interface.h"
#include "x86-nxvm/board-base/board_profile_interface.h"

#define MYPC110_FIRMWARE_BYTES (256u * 1024u)
#define MYPC110_FIRMWARE_BANK_BYTES (64u * 1024u)
#define MYPC110_VIDEO_PHYSICAL_START 0x000c0000u
#define MYPC110_RESET_ROMSET 0x29u
#define MYPC110_SMRAM_PHYSICAL_START 0x00030000u
#define MYPC110_SMRAM_BYTES (64u * 1024u)

/* One App-owned PC110 firmware map and its SL-compatible decode controls.
 * CPU configuration lock/mode is distinct from the chip's Block 2 unlock.
 * Flash is immutable. One cold-zero shadow holds C--F; chipset access fields
 * select reads/writes independently. Guest firmware performs all shadow copies. */
typedef struct mypc110_firmware {
    const lib_u8 *bios;
    lib_u8 *shadow;
    x86_vl82c420 *configuration_chip;
    x86_core_machine *machine;
    lib_u8 cpu_unlock_step;
    lib_u8 planar_control;
    lib_u16 cpu_power_mode;
    lib_u16 memory_control;
    lib_bool smram_active;
} mypc110_firmware;

lib_status mypc110_firmware_create(const lib_u8 *bios, lib_size bytes,
    mypc110_firmware *out_firmware);
void mypc110_firmware_destroy(mypc110_firmware *firmware);
const x86_core_machine_firmware_provider *mypc110_firmware_provider(void);
lib_status mypc110_firmware_install_routes(x86_core_machine *machine,
    mypc110_firmware *firmware);
void mypc110_firmware_system_management_active(void *context, lib_bool active);
/* Physical button input, not monitor resume. Call only on the owning executor
 * while the binding is live; a qualified edge requests retained CPU RESET. */
lib_status mypc110_firmware_set_resume_button(mypc110_firmware *firmware,
    lib_bool asserted);
lib_status mypc110_firmware_profile_create(const x86_nxvm_board_profile_services *services,
    void *context, x86_nxvm_board_profile_binding *out_binding);

#endif
