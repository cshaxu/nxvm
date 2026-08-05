#ifndef VM_PROFILE_DEFAULT_MOUSE_MAPPER_H
#define VM_PROFILE_DEFAULT_MOUSE_MAPPER_H

#include "type.h"

typedef struct vm_profile_default_mouse_report {
    int16_t delta_x;
    int16_t delta_y;
    uint8_t buttons;
} vm_profile_default_mouse_report;

/* The profile translates host-relative coordinates into PS/2 convention. The
 * KBC remains the sole owner of packet bytes and guest-visible state. */
type_status vm_profile_default_mouse_map_host_relative(int16_t host_delta_x,
    int16_t host_delta_y, uint8_t host_buttons,
    vm_profile_default_mouse_report *out_report);

#endif
