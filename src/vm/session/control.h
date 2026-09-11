#ifndef VM_SESSION_CONTROL_H
#define VM_SESSION_CONTROL_H

#include "type.h"

#include "lib/ui-base/event_interface.h"
#include "vm/events/machine_event.h"

typedef struct vm_session_control vm_session_control;

typedef enum vm_session_fact_kind {
    VM_SESSION_FACT_CONSOLE_LINE,
    VM_SESSION_FACT_MACHINE_RESULT,
    VM_SESSION_FACT_PRESENTATION_INPUT,
    VM_SESSION_FACT_DISPLAY
} vm_session_fact_kind;

typedef struct vm_session_fact {
    vm_session_fact_kind kind;
    type_unsigned_32 run_generation;
    union {
        C_CHAR line[1024];
        struct {
            vm_machine_result_kind kind;
            type_status status;
        } machine;
        ui_input_event presentation_input;
    } value;
} vm_session_fact;

typedef type_status (*vm_session_control_input_sink)(C_VOID *context,
    const ui_input_event *event);

type_status vm_session_control_create(vm_session_control **out_control);
C_VOID vm_session_control_destroy(vm_session_control *control);
type_status vm_session_control_publish_console_line(vm_session_control *control,
    const C_CHAR *line);
type_status vm_session_control_publish_presentation_input(vm_session_control *control,
    const ui_input_event *event);
type_status vm_session_control_publish_machine_result(vm_session_control *control,
    const vm_machine_result *result);
/* This is the one copied machine-result ingress.  vm/machine retains only
 * the control FIFO context, never the owning vm/session object. */
C_VOID vm_session_control_machine_result_sink(C_VOID *context,
    const vm_machine_result *result);
type_unsigned_32 vm_session_control_begin_run(vm_session_control *control);
type_status vm_session_control_take(vm_session_control *control,
    vm_session_fact *out_fact, vm_machine_display_event *out_display,
    type_unsigned_32 timeout_milliseconds);
C_VOID vm_session_control_close(vm_session_control *control);
const C_CHAR *vm_session_control_note_machine_result(vm_session_control *control,
    vm_machine_result_kind kind);
type_status vm_session_control_dispatch_host_input(vm_session_control *control,
    const ui_input_event *event, C_INT machine_running,
    vm_session_control_input_sink sink, C_VOID *sink_context);

#endif
