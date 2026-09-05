#ifndef VM_PLATFORM_UX_FRAME_H
#define VM_PLATFORM_UX_FRAME_H

#include "type.h"
#include "core/platform/display_frame.h"
#include "lib/ux/frame.h"

/* VM owns this representation conversion. Neither Core nor lib/ux depends on
 * the other's frame format. */
type_status vm_platform_ux_frame_from_core(
    const core_platform_display_frame *source, ux_frame *destination);

#endif
