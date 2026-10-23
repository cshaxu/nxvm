#ifndef MYPC110_MACHINE_INTERFACE_H
#define MYPC110_MACHINE_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-core/chips/vl82c420/vl82c420_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"

typedef struct mypc110_base_composition {
    x86_core_machine *core;
    x86_nxvm_board_state *board;
    x86_vl82c420 *configuration_chip;
} mypc110_base_composition;

/* Creates only the source-backed base: 486SX Core, standard controllers,
 * TFT firmware map and VL82C420 routes.  It intentionally has no reset alias,
 * input, panel, storage, RTC seed or execution-provider contract. */
lib_status mypc110_base_composition_create(const lib_u8 *tft_bios,
    lib_size tft_bios_bytes, mypc110_base_composition *out_composition);
void mypc110_base_composition_destroy(mypc110_base_composition *composition);

#endif
