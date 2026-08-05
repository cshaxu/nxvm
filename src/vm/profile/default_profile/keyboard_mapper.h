#ifndef VM_PROFILE_DEFAULT_KEYBOARD_MAPPER_H
#define VM_PROFILE_DEFAULT_KEYBOARD_MAPPER_H

#include "type.h"

/* The profile maps one host transition into a bounded set-1 byte sequence.
 * The core KBC remains the sole guest-visible FIFO. */
#define VM_PROFILE_DEFAULT_KEYBOARD_SEQUENCE_CAPACITY 6u

typedef struct vm_profile_default_keyboard_sequence {
    uint8_t bytes[VM_PROFILE_DEFAULT_KEYBOARD_SEQUENCE_CAPACITY];
    uint8_t count;
} vm_profile_default_keyboard_sequence;

type_status vm_profile_default_keyboard_map_host_key(uint16_t host_scan_code,
    uint16_t host_virtual_key, C_INT pressed,
    vm_profile_default_keyboard_sequence *out_sequence);

#endif
