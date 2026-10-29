#ifndef MYPC110_PRODUCT_VDISPLAY_H
#define MYPC110_PRODUCT_VDISPLAY_H

#include "lib/types/atomic.h"
#include "x86-ibmpc/product/command_interface.h"
#include "x86-nxvm/machine/construction_interface.h"
#include "x86-nxvm/machine/input_interface.h"

typedef enum mypc110_display_variant {
    MYPC110_DISPLAY_DSTN,
    MYPC110_DISPLAY_TFT
} mypc110_display_variant;

typedef struct mypc110_vdisplay {
    lib_atomic_i32 requested;
    mypc110_display_variant active;
    const x86_nxvm_machine_assets *firmware[2];
    lib_u8 cmos[2][X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES];
    lib_bool present[2];
    lib_bool dirty[2];
    x86_ibmpc_product_command_extensions standard;
    emulator_product_help_row help[8];
} mypc110_vdisplay;

void mypc110_vdisplay_initialize(mypc110_vdisplay *display);
lib_bool mypc110_vdisplay_submit(void *context, emulator_machine *machine,
    emulator_session_machine_state state, const char *line,
    emulator_session_command_result *out);

#endif
