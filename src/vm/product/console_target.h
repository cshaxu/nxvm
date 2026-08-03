#ifndef NTVDM64_VM_PRODUCT_CONSOLE_TARGET_H
#define NTVDM64_VM_PRODUCT_CONSOLE_TARGET_H

#include "type.h"

#include <stddef.h>

#include <stdint.h>

typedef struct nxvm_product_console_target {
    C_VOID (*initialize)(C_VOID *context);
    C_VOID (*finalize)(C_VOID *context);
    C_INT (*is_running)(C_VOID *context);
    C_VOID (*print_machine)(C_VOID *context);
    C_INT (*get_window_display)(C_VOID *context);
    C_VOID (*set_window_display)(C_VOID *context, C_INT enabled);
    C_VOID (*print_bios)(C_VOID *context);
    C_VOID (*print_status)(C_VOID *context);
    C_VOID (*debug)(C_VOID *context);
    C_VOID (*record_start)(C_VOID *context, const C_CHAR *path);
    C_VOID (*record_stop)(C_VOID *context);
    C_VOID (*set_boot_hdd)(C_VOID *context, C_INT enabled);
    C_VOID (*set_memory)(C_VOID *context, STD_SIZE_T bytes);
    C_VOID (*create_fdd)(C_VOID *context);
    C_INT (*insert_fdd)(C_VOID *context, const C_CHAR *path);
    C_INT (*remove_fdd)(C_VOID *context, const C_CHAR *path);
    C_VOID (*create_hdd)(C_VOID *context, uint16_t cylinders);
    C_INT (*insert_hdd)(C_VOID *context, const C_CHAR *path);
    C_INT (*remove_hdd)(C_VOID *context, const C_CHAR *path);
    C_VOID (*start)(C_VOID *context);
    C_VOID (*reset)(C_VOID *context);
    C_VOID (*stop)(C_VOID *context);
    C_VOID (*resume)(C_VOID *context);
    C_VOID *context;
} nxvm_product_console_target;

#endif
