#ifndef VM_APP_MACHINE_INTERFACE_H
#define VM_APP_MACHINE_INTERFACE_H

#include "lib/types/types_interface.h"
#include "common/machine/machine_interface.h"
#include "x86/product/request_interface.h"

typedef enum vm_app_speed {
    VM_APP_SPEED_STANDARD,
    VM_APP_SPEED_TURBO
} vm_app_speed;

typedef struct vm_app_information {
    const char *machine_name;
    const char *cpu_name;
    lib_u64 memory_bytes;
    lib_u64 floppy_image_bytes;
    lib_bool floppy_media_inserted;
    lib_bool fixed_disk_present;
    lib_u32 fixed_disk_cylinders;
    lib_u64 fixed_disk_image_bytes;
    lib_bool fixed_disk_media_connected;
    lib_bool external_firmware;
} vm_app_information;

/* Copied once at creation. Context and INFO names are immutable borrowed
 * values that outlive Product. Prepare publishes a complete candidate/driver
 * or clears its output and releases all partial construction. Product owns
 * Common shutdown before revoking and destroying the App-owned candidate. */
typedef struct vm_app_factory {
    const void *context;
    lib_status (*prepare)(const void *context, const vm_session_request *request,
        void **out_machine, common_machine_driver *out_driver);
    lib_status (*bind)(void *machine, common_machine *common);
    void (*destroy)(void *machine);
    lib_status (*information)(const void *machine, vm_app_information *out_info);
    lib_status (*get_speed)(const void *machine, vm_app_speed *out_speed);
    lib_status (*set_speed)(void *machine, vm_app_speed speed);
} vm_app_factory;

#endif
