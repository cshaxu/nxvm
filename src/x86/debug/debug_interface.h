#ifndef X86_DEBUG_INTERFACE_H
#define X86_DEBUG_INTERFACE_H

#include "lib/types/types_interface.h"
#include "common/machine/machine_interface.h"
#include "x86/debug/protocol_interface.h"

typedef struct x86_debug x86_debug;


#define X86_DEBUG_LINE_CAPACITY 256u
#define X86_DEBUG_PROMPT_CAPACITY 64u

typedef enum x86_debug_lifecycle_request {
    X86_DEBUG_LIFECYCLE_NONE,
    X86_DEBUG_LIFECYCLE_RESUME,
    X86_DEBUG_LIFECYCLE_STEP,
    X86_DEBUG_LIFECYCLE_STOP
} x86_debug_lifecycle_request;

typedef struct x86_debug_result {
    /* Borrowed, NUL-terminated output, valid until the next submit/observe,
     * open or destroy on this debug object. Copy before retaining longer.
     * Output grows as needed; allocation/format failure returns lib_status. */
    const char *text;
    char prompt[X86_DEBUG_PROMPT_CAPACITY];
    lib_bool prompt_ready;
    lib_bool keep_active;
    x86_debug_lifecycle_request lifecycle_request;
} x86_debug_result;

typedef enum x86_debug_machine_state {
    X86_DEBUG_MACHINE_RUNNING,
    X86_DEBUG_MACHINE_PAUSED,
    X86_DEBUG_MACHINE_RESET,
    X86_DEBUG_MACHINE_STOPPED,
    X86_DEBUG_MACHINE_FAULT
} x86_debug_machine_state;

lib_status x86_debug_create(x86_debug **out_debug);
void x86_debug_destroy(x86_debug *debug);
lib_status x86_debug_open(x86_debug *debug, common_machine *machine);
void x86_debug_close(x86_debug *debug);
lib_status x86_debug_submit_line(x86_debug *debug, const char *line,
    x86_debug_result *out_result);
lib_status x86_debug_observe_machine(x86_debug *debug,
    x86_debug_machine_state state, lib_status status,
    x86_debug_result *out_result);

#endif
