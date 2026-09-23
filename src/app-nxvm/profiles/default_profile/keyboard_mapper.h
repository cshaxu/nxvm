#ifndef VM_PROFILE_DEFAULT_KEYBOARD_MAPPER_H
#define VM_PROFILE_DEFAULT_KEYBOARD_MAPPER_H

#include "type.h"

/* The profile adapts one host transition into the selected keyboard's native
 * Set-2 serial stream. The core KBC remains the sole guest-visible FIFO and
 * performs any 8042 Set-2-to-Set-1 translation. */
#define VM_PROFILE_DEFAULT_KEYBOARD_SEQUENCE_CAPACITY 8u

typedef struct vm_profile_default_keyboard_sequence {
    type_unsigned_8 bytes[VM_PROFILE_DEFAULT_KEYBOARD_SEQUENCE_CAPACITY];
    type_unsigned_8 count;
} vm_profile_default_keyboard_sequence;

type_status vm_profile_default_keyboard_map_host_key(type_unsigned_16 host_scan_code,
    type_unsigned_16 host_virtual_key, C_INT pressed,
    vm_profile_default_keyboard_sequence *out_sequence);
type_status vm_profile_default_keyboard_map_host_key_for_scan_set(
    type_unsigned_16 host_scan_code, type_unsigned_16 host_virtual_key,
    C_INT pressed, type_unsigned_8 native_scan_set,
    vm_profile_default_keyboard_sequence *out_sequence);

#endif
