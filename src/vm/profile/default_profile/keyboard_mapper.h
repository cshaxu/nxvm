#ifndef VM_PROFILE_DEFAULT_KEYBOARD_MAPPER_H
#define VM_PROFILE_DEFAULT_KEYBOARD_MAPPER_H

#include "type.h"

/* Converts a host-neutral event to the default profile's set-1 make byte. */
type_status vm_profile_default_keyboard_map_host_key(uint16_t host_scan_code,
    uint16_t host_virtual_key, uint8_t *out_scan_code);

#endif
