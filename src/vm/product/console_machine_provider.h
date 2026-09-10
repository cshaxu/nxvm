#ifndef VM_PRODUCT_CONSOLE_MACHINE_PROVIDER_H
#define VM_PRODUCT_CONSOLE_MACHINE_PROVIDER_H

#include "type.h"

#include "vm/composition/session/session_state.h"
#include "core/machine/guest_display_frame.h"
#include "core/machine/guest_input_interface.h"

typedef struct vm_product_session_request vm_product_session_request;

typedef enum vm_product_console_speed {
    VM_PRODUCT_CONSOLE_SPEED_STANDARD,
    VM_PRODUCT_CONSOLE_SPEED_TURBO
} vm_product_console_speed;

typedef C_VOID (*vm_product_console_lifecycle_reporter)(C_VOID *context,
    vm_session_lifecycle lifecycle);
typedef C_VOID (*vm_product_console_display_reporter)(C_VOID *context,
    const core_machine_guest_display_frame *frame);

typedef struct vm_session_machine_provider {
    C_INT (*is_running)(C_VOID *context);
    C_VOID (*print_machine)(C_VOID *context);
    C_VOID (*set_lifecycle_reporter)(C_VOID *context,
        vm_product_console_lifecycle_reporter reporter, C_VOID *reporter_context);
    C_VOID (*set_display_reporter)(C_VOID *context,
        vm_product_console_display_reporter reporter, C_VOID *reporter_context);
    C_VOID (*print_bios)(C_VOID *context);
    C_VOID (*print_status)(C_VOID *context);
    type_status (*get_speed)(C_VOID *context, vm_product_console_speed *out_speed);
    type_status (*set_speed)(C_VOID *context, vm_product_console_speed speed);
    C_VOID (*debug)(C_VOID *context);
    C_VOID (*record_start)(C_VOID *context, const C_CHAR *path);
    C_VOID (*record_stop)(C_VOID *context);
    C_INT (*insert_fdd)(C_VOID *context, const C_CHAR *path);
    C_INT (*remove_fdd)(C_VOID *context, const C_CHAR *path);
    type_status (*open_profile)(C_VOID *context,
        const vm_product_session_request *request);
    type_status (*start)(C_VOID *context);
    C_VOID (*reset)(C_VOID *context);
    type_status (*stop)(C_VOID *context);
    type_status (*resume)(C_VOID *context);
    type_status (*request_pause)(C_VOID *context);
    type_status (*submit_host_input)(C_VOID *context,
        const core_machine_guest_input_event *event);
    C_VOID *context;
} vm_session_machine_provider;

#endif
