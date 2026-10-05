#ifndef VM_MACHINE_FRAME_H
#define VM_MACHINE_FRAME_H
#include "lib/types/types_interface.h"


#include "common/machine/frame_interface.h"
#include "x86/chips/video/video_values_interface.h"

/* Pure copied-snapshot conversion; the adapter supplies its presentation
 * sequence. Failure preserves the destination. No capture or guest mutation. */
lib_status vm_machine_frame_from_display(
    const x86_video_snapshot *source, lib_u64 sequence,
    common_machine_frame *destination);

#endif
