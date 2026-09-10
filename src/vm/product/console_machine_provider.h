#ifndef VM_PRODUCT_CONSOLE_MACHINE_PROVIDER_H
#define VM_PRODUCT_CONSOLE_MACHINE_PROVIDER_H

#include "type.h"

#include "core/product/session/session_interface.h"
#include "vm/platform/session_state.h"

struct vm_platform_console_binding;

typedef enum vm_session_display_mode {
    VM_SESSION_DISPLAY_CONSOLE,
    VM_SESSION_DISPLAY_WINDOW
} vm_session_display_mode;

typedef enum vm_product_console_speed {
    VM_PRODUCT_CONSOLE_SPEED_STANDARD,
    VM_PRODUCT_CONSOLE_SPEED_TURBO
} vm_product_console_speed;

typedef C_VOID (*vm_product_console_lifecycle_reporter)(C_VOID *context,
    core_product_session_id id, vm_session_lifecycle lifecycle);

typedef struct vm_session_machine_provider {
    C_INT (*is_running)(C_VOID *context);
    C_VOID (*print_machine)(C_VOID *context);
    vm_session_display_mode (*get_display_mode)(C_VOID *context);
    C_VOID (*set_display_mode)(C_VOID *context, vm_session_display_mode mode);
    C_VOID (*set_console_binding)(C_VOID *context,
        const struct vm_platform_console_binding *binding);
    C_VOID (*set_lifecycle_reporter)(C_VOID *context,
        vm_product_console_lifecycle_reporter reporter, C_VOID *reporter_context);
    C_VOID (*print_bios)(C_VOID *context);
    C_VOID (*print_status)(C_VOID *context);
    type_status (*get_speed)(C_VOID *context, vm_product_console_speed *out_speed);
    type_status (*set_speed)(C_VOID *context, vm_product_console_speed speed);
    C_VOID (*debug)(C_VOID *context);
    C_VOID (*record_start)(C_VOID *context, const C_CHAR *path);
    C_VOID (*record_stop)(C_VOID *context);
    C_INT (*insert_fdd)(C_VOID *context, const C_CHAR *path);
    C_INT (*remove_fdd)(C_VOID *context, const C_CHAR *path);
    type_status (*start)(C_VOID *context);
    C_VOID (*reset)(C_VOID *context);
    C_VOID (*stop)(C_VOID *context);
    type_status (*stop_all)(C_VOID *context);
    type_status (*resume)(C_VOID *context);
    C_VOID *context;
} vm_session_machine_provider;

#endif
