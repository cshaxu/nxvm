#ifndef X86_NXVM_MACHINE_MOUSE_MAPPER_H
#define X86_NXVM_MACHINE_MOUSE_MAPPER_H
#include "lib/types/types_interface.h"


typedef struct x86_nxvm_machine_mouse_report {
    lib_i16 delta_x;
    lib_i16 delta_y;
    lib_u8 buttons;
} x86_nxvm_machine_mouse_report;

/* Pure conversion translates host-relative coordinates into PS/2 convention. The
 * KBC remains the sole owner of packet bytes and guest-visible state. */
lib_status x86_nxvm_machine_mouse_map_host_relative(lib_i16 host_delta_x,
    lib_i16 host_delta_y, lib_u8 host_buttons,
    x86_nxvm_machine_mouse_report *out_report);

#endif
