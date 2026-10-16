#ifndef X86_NXVM_MACHINE_FRAME_H
#define X86_NXVM_MACHINE_FRAME_H
#include "lib/types/types_interface.h"


#include "emulator/machine/frame_interface.h"
#include "x86-nxvm/chips/video/video_values_interface.h"

/* Pure copied-snapshot conversion; the adapter supplies its presentation
 * sequence. Failure preserves the destination. No capture or guest mutation. */
lib_status x86_nxvm_machine_frame_from_display(
    const x86_video_snapshot *source, lib_u64 sequence,
    emulator_machine_frame *destination);

#endif
