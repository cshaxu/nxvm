#ifndef MYPC110_FIRMWARE_INTERFACE_H
#define MYPC110_FIRMWARE_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-core/chips/vl82c420/vl82c420_interface.h"
#include "x86-core/machine/firmware_interface.h"

#define MYPC110_FIRMWARE_BYTES (256u * 1024u)
#define MYPC110_FIRMWARE_BANK_BYTES (64u * 1024u)
#define MYPC110_VIDEO_SHADOW_BYTES (48u * 1024u)
#define MYPC110_VIDEO_PHYSICAL_START 0x000c0000u

/* One App-owned PC110 firmware map. The selected immutable Flash image remains immutable;
 * the C000 video image is always the selected video bank, while the observed
 * EC/ED sequence controls whether that shadow may be patched. */
typedef struct mypc110_firmware {
    const lib_u8 *bios;
    lib_u8 *video_shadow;
    lib_u8 *system_shadow;
    x86_vl82c420 *configuration_chip;
    lib_u8 eced_index;
    lib_bool eced_config_open;
    lib_bool video_shadow_writable;
} mypc110_firmware;

lib_status mypc110_firmware_create(const lib_u8 *bios, lib_size bytes,
    mypc110_firmware *out_firmware);
void mypc110_firmware_destroy(mypc110_firmware *firmware);
const x86_core_machine_firmware_provider *mypc110_firmware_provider(void);
lib_status mypc110_firmware_install_routes(x86_core_machine *machine,
    mypc110_firmware *firmware);
void mypc110_firmware_note_eced_write(mypc110_firmware *firmware,
    lib_u8 index, lib_u8 value);

#endif
