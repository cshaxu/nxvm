#ifndef VM_PROFILE_DEFAULT_MOUSE_MAPPER_H
#define VM_PROFILE_DEFAULT_MOUSE_MAPPER_H
#include "lib/types/types_interface.h"


typedef struct vm_profile_default_mouse_report {
    lib_i16 delta_x;
    lib_i16 delta_y;
    lib_u8 buttons;
} vm_profile_default_mouse_report;

/* The profile translates host-relative coordinates into PS/2 convention. The
 * KBC remains the sole owner of packet bytes and guest-visible state. */
lib_status vm_profile_default_mouse_map_host_relative(lib_i16 host_delta_x,
    lib_i16 host_delta_y, lib_u8 host_buttons,
    vm_profile_default_mouse_report *out_report);

#endif
