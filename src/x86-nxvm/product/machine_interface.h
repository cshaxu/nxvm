#ifndef X86_NXVM_PRODUCT_MACHINE_INTERFACE_H
#define X86_NXVM_PRODUCT_MACHINE_INTERFACE_H

#include "emulator/product/machine_interface.h"

/* NXVM-specific information and speed contracts; no runtime implementation. */
typedef enum x86_nxvm_product_speed {
    X86_NXVM_PRODUCT_SPEED_STANDARD,
    X86_NXVM_PRODUCT_SPEED_TURBO
} x86_nxvm_product_speed;

typedef struct x86_nxvm_product_information {
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
} x86_nxvm_product_information;

typedef struct x86_nxvm_product_machine {
    emulator_product_machine composition;
    const void *context;
    lib_status (*information)(const void *context, const void *machine,
        x86_nxvm_product_information *out_info);
    lib_status (*get_speed)(const void *machine, x86_nxvm_product_speed *out_speed);
    lib_status (*set_speed)(void *machine, x86_nxvm_product_speed speed);
} x86_nxvm_product_machine;

#endif
