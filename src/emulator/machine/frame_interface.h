#ifndef EMULATOR_MACHINE_FRAME_INTERFACE_H
#define EMULATOR_MACHINE_FRAME_INTERFACE_H

#include "lib/kvm-window/frame_interface.h"
#include "lib/kvm-console/frame_interface.h"

/* One upstream publication: the Window representation plus text-only Console
 * resources. Graphics do not copy or interpret characters. Sequence belongs to
 * this publication, independently of each leaf mailbox's acknowledgement. */
typedef struct emulator_machine_frame {
    lib_u32 sequence;
    kvm_window_frame window;
    kvm_console_character_map characters;
} emulator_machine_frame;

static inline lib_bool emulator_machine_frame_copy(emulator_machine_frame *destination,
    const emulator_machine_frame *source)
{
    if (destination == LIB_NULL || source == LIB_NULL ||
        !kvm_window_frame_copy(&destination->window, &source->window)) return LIB_FALSE;
    if (destination != source) {
        destination->sequence = source->sequence;
        if (source->window.graphics == 0u)
            destination->characters = source->characters;
    }
    return LIB_TRUE;
}

#endif
