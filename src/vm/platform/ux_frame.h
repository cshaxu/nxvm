#ifndef VM_PLATFORM_UX_FRAME_H
#define VM_PLATFORM_UX_FRAME_H

#include "type.h"
#include "core/machine/guest_display_frame.h"
#include "lib/ux-base/frame_interface.h"

/* VM owns this representation conversion. Neither Core nor lib/ux depends on
 * the other's frame format. */
type_status vm_platform_ux_frame_from_core(
    const core_machine_guest_display_frame *source, ux_frame *destination);

#endif
