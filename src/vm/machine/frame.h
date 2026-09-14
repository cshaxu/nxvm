#ifndef VM_MACHINE_FRAME_H
#define VM_MACHINE_FRAME_H

#include "type.h"

#include "lib/kvm-base/frame_interface.h"
#include "vm/machine/event_interface.h"

/* The NXVM Core adapter owns this copied-value conversion from its guest
 * display result into the platform-neutral Lib frame ABI. */
type_status vm_machine_frame_from_display(
    const vm_machine_display_event *source, kvm_frame *destination);

#endif
