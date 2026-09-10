#ifndef VM_PRODUCT_PRESENTATION_FRAME_H
#define VM_PRODUCT_PRESENTATION_FRAME_H

#include "type.h"

#include "core/machine/guest_display_frame.h"
#include "lib/ux-base/frame_interface.h"

/* The product owns this one copied-value conversion from Core's guest
 * snapshot into the platform-neutral lib frame ABI. */
type_status vm_product_presentation_frame_from_core(
    const core_machine_guest_display_frame *source, ux_frame *destination);

#endif
