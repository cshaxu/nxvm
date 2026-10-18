#ifndef X86_NXVM_MACHINE_FRAME_INTERFACE_H
#define X86_NXVM_MACHINE_FRAME_INTERFACE_H
#include "lib/types/types_interface.h"


#include "lib/kvm-window/frame_interface.h"
#include "lib/kvm-console/frame_interface.h"
#include "x86-nxvm/chips/video/video_values_interface.h"

/* Pure copied-snapshot conversion; the adapter supplies its presentation
 * sequence. Failure preserves the destination. No capture or guest mutation. */
lib_status x86_nxvm_machine_frame_from_display(
    const x86_video_snapshot *source, lib_u64 sequence,
    kvm_window_frame_storage *destination, kvm_console_text_frame *console);

#endif
