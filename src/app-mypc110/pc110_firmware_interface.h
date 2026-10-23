#ifndef MYPC110_FIRMWARE_INTERFACE_H
#define MYPC110_FIRMWARE_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-core/machine/firmware_interface.h"

#define MYPC110_TFT_BIOS_BYTES (256u * 1024u)
#define MYPC110_TFT_BIOS_PHYSICAL_START 0x000c0000u

/* Construction-only binding for the qualified TFT BIOS image. Core copies the
 * bytes while configuring the provider; callers retain no firmware lease. */
typedef struct mypc110_tft_firmware {
    const lib_u8 *bios;
} mypc110_tft_firmware;

lib_status mypc110_tft_firmware_create(const lib_u8 *bios, lib_size bytes,
    mypc110_tft_firmware *out_firmware);
const x86_core_machine_firmware_provider *mypc110_tft_firmware_provider(void);

#endif
