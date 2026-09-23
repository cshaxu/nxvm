#ifndef VM_PROFILE_DEFAULT_MOUSE_MAPPER_H
#define VM_PROFILE_DEFAULT_MOUSE_MAPPER_H

#include "type.h"

typedef struct vm_profile_default_mouse_report {
    type_signed_16 delta_x;
    type_signed_16 delta_y;
    type_unsigned_8 buttons;
} vm_profile_default_mouse_report;

/* The profile translates host-relative coordinates into PS/2 convention. The
 * KBC remains the sole owner of packet bytes and guest-visible state. */
type_status vm_profile_default_mouse_map_host_relative(type_signed_16 host_delta_x,
    type_signed_16 host_delta_y, type_unsigned_8 host_buttons,
    vm_profile_default_mouse_report *out_report);

#endif
