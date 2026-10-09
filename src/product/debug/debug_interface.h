#ifndef PRODUCT_DEBUG_INTERFACE_H
#define PRODUCT_DEBUG_INTERFACE_H

#include "lib/types/types_interface.h"
#include "emulator/machine/machine_interface.h"
#include "product/debug/protocol_interface.h"

typedef struct product_debug product_debug;


#define PRODUCT_DEBUG_LINE_CAPACITY 256u
#define PRODUCT_DEBUG_PROMPT_CAPACITY 64u

typedef enum product_debug_lifecycle_request {
    PRODUCT_DEBUG_LIFECYCLE_NONE,
    PRODUCT_DEBUG_LIFECYCLE_RESUME,
    PRODUCT_DEBUG_LIFECYCLE_STEP,
    PRODUCT_DEBUG_LIFECYCLE_STOP
} product_debug_lifecycle_request;

typedef struct product_debug_result {
    /* Borrowed, NUL-terminated output without a terminal line ending. It is valid
     * until the next submit/observe, open, or destroy on this debug object. Copy before retaining longer.
     * Output grows as needed; allocation/format failure returns lib_status. */
    const char *text;
    char prompt[PRODUCT_DEBUG_PROMPT_CAPACITY];
    lib_bool prompt_ready;
    lib_bool keep_active;
    product_debug_lifecycle_request lifecycle_request;
} product_debug_result;

typedef enum product_debug_machine_state {
    PRODUCT_DEBUG_MACHINE_RUNNING,
    PRODUCT_DEBUG_MACHINE_PAUSED,
    PRODUCT_DEBUG_MACHINE_RESET,
    PRODUCT_DEBUG_MACHINE_STOPPED,
    PRODUCT_DEBUG_MACHINE_FAULT
} product_debug_machine_state;

lib_status product_debug_create(product_debug **out_debug);
void product_debug_destroy(product_debug *debug);
lib_status product_debug_open(product_debug *debug, emulator_machine *machine);
void product_debug_close(product_debug *debug);
lib_status product_debug_submit_line(product_debug *debug, const char *line,
    product_debug_result *out_result);
lib_status product_debug_observe_machine(product_debug *debug,
    product_debug_machine_state state, lib_status status,
    product_debug_result *out_result);

#endif
