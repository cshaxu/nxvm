#ifndef VM_PRODUCT_CONTROL_H
#define VM_PRODUCT_CONTROL_H

#include "type.h"

#include "vm/machine/runtime/executor_state.h"
#include "core/machine/guest_display_frame.h"
#include "lib/ui-base/event_interface.h"

typedef struct vm_product_control vm_product_control;

typedef type_status (*vm_product_control_input_sink)(C_VOID *context,
    const ui_input_event *event);

typedef enum vm_product_control_fact_kind {
    VM_PRODUCT_CONTROL_FACT_MONITOR_LINE,
    VM_PRODUCT_CONTROL_FACT_LIFECYCLE,
    VM_PRODUCT_CONTROL_FACT_HOST_INPUT,
    VM_PRODUCT_CONTROL_FACT_DISPLAY
} vm_product_control_fact_kind;

typedef struct vm_product_control_fact {
    vm_product_control_fact_kind kind;
    type_unsigned_32 run_generation;
    union {
        C_CHAR line[1024];
        vm_machine_lifecycle lifecycle;
        ui_input_event host_input;
    } value;
} vm_product_control_fact;

type_status vm_product_control_create(vm_product_control **out_control);
C_VOID vm_product_control_destroy(vm_product_control *control);
type_status vm_product_control_publish(vm_product_control *control,
    const vm_product_control_fact *fact);
type_status vm_product_control_publish_display(vm_product_control *control,
    const core_machine_guest_display_frame *frame);
C_UINT vm_product_control_begin_run(vm_product_control *control);
/* Product-owned input hygiene: a retiring UX source releases the physical
 * keys which this product actually delivered while the session was running.
 * The callback is deliberately neutral; it is the product's job to translate
 * a UX event into a guest event. */
type_status vm_product_control_dispatch_host_input(vm_product_control *control,
    const ui_input_event *event, C_INT session_running,
    vm_product_control_input_sink sink, C_VOID *sink_context);
type_status vm_product_control_take(vm_product_control *control,
    vm_product_control_fact *out_fact,
    core_machine_guest_display_frame *out_display,
    C_UINT timeout_milliseconds);
C_VOID vm_product_control_close(vm_product_control *control);
const C_CHAR *vm_product_control_note_lifecycle(vm_product_control *control,
    vm_machine_lifecycle lifecycle);

#endif
