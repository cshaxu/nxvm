#ifndef MYPC110_FIRMWARE_INTERFACE_H
#define MYPC110_FIRMWARE_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-core/machine/firmware_interface.h"

#define MYPC110_TFT_BIOS_BYTES (256u * 1024u)
#define MYPC110_TFT_FIRMWARE_BANK_BYTES (64u * 1024u)

/* One App-owned PC110 firmware map. The TFT Flash image remains immutable;
 * only the hardware-visible F-segment shadow is writable. */
typedef struct mypc110_tft_firmware {
    const lib_u8 *bios;
    lib_u8 *system_shadow;
} mypc110_tft_firmware;

lib_status mypc110_tft_firmware_create(const lib_u8 *bios, lib_size bytes,
    mypc110_tft_firmware *out_firmware);
void mypc110_tft_firmware_destroy(mypc110_tft_firmware *firmware);
const x86_core_machine_firmware_provider *mypc110_tft_firmware_provider(void);
lib_status mypc110_tft_firmware_install_routes(x86_core_machine *machine,
    mypc110_tft_firmware *firmware);

#endif
