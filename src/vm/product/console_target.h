#ifndef NTVDM64_VM_PRODUCT_CONSOLE_TARGET_H
#define NTVDM64_VM_PRODUCT_CONSOLE_TARGET_H

#include <stddef.h>
#include <stdint.h>

typedef struct nxvm_product_console_target {
    void (*initialize)(void *context);
    void (*finalize)(void *context);
    int (*is_running)(void *context);
    void (*print_machine)(void *context);
    int (*get_window_display)(void *context);
    void (*set_window_display)(void *context, int enabled);
    void (*print_bios)(void *context);
    void (*print_status)(void *context);
    void (*debug)(void *context);
    void (*record_start)(void *context, const char *path);
    void (*record_stop)(void *context);
    void (*set_boot_hdd)(void *context, int enabled);
    void (*set_memory)(void *context, size_t bytes);
    void (*create_fdd)(void *context);
    int (*insert_fdd)(void *context, const char *path);
    int (*remove_fdd)(void *context, const char *path);
    void (*create_hdd)(void *context, uint16_t cylinders);
    int (*insert_hdd)(void *context, const char *path);
    int (*remove_hdd)(void *context, const char *path);
    void (*start)(void *context);
    void (*reset)(void *context);
    void (*stop)(void *context);
    void (*resume)(void *context);
    void *context;
} nxvm_product_console_target;

#endif
