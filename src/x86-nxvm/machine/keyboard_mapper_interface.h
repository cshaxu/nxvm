#ifndef X86_NXVM_MACHINE_KEYBOARD_MAPPER_H
#define X86_NXVM_MACHINE_KEYBOARD_MAPPER_H
#include "lib/types/types_interface.h"
#include "lib/kvm-base/event_interface.h"


/* Pure conversion adapts one neutral KVM key transition into the selected
 * keyboard's native serial stream. The core KBC remains the sole
 * guest-visible FIFO and performs any 8042 Set-2-to-Set-1 translation. */
#define X86_NXVM_MACHINE_KEYBOARD_SEQUENCE_CAPACITY 8u

typedef struct x86_nxvm_machine_keyboard_sequence {
    lib_u8 bytes[X86_NXVM_MACHINE_KEYBOARD_SEQUENCE_CAPACITY];
    lib_u8 count;
} x86_nxvm_machine_keyboard_sequence;

lib_status x86_nxvm_machine_keyboard_map_kvm_event(
    const kvm_input_event *event,
    x86_nxvm_machine_keyboard_sequence *out_sequence);
lib_status x86_nxvm_machine_keyboard_map_kvm_event_for_scan_set(
    const kvm_input_event *event, lib_u8 native_scan_set,
    x86_nxvm_machine_keyboard_sequence *out_sequence);

#endif
