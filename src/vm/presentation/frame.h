#ifndef VM_PRESENTATION_FRAME_H
#define VM_PRESENTATION_FRAME_H

#include "type.h"

#include "lib/ui-base/frame_interface.h"
#include "vm/events/machine_event.h"

/* The product owns this one copied-value conversion from Core's guest
 * snapshot into the platform-neutral lib frame ABI. */
type_status vm_presentation_frame_from_core(
    const vm_machine_display_event *source, ui_frame *destination);

#endif
