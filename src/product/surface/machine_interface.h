#ifndef PRODUCT_SURFACE_MACHINE_INTERFACE_H
#define PRODUCT_SURFACE_MACHINE_INTERFACE_H

#include "emulator/product/machine_interface.h"

/* These are IBM PC App extension values, not Emulator Product facts. */
typedef enum product_surface_speed {
    PRODUCT_SURFACE_SPEED_STANDARD,
    PRODUCT_SURFACE_SPEED_TURBO
} product_surface_speed;

typedef struct product_surface_information {
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
} product_surface_information;

typedef struct app_composed_machine {
    emulator_product_machine composition;
    const void *context;
    lib_status (*information)(const void *context, const void *machine,
        product_surface_information *out_info);
    lib_status (*get_speed)(const void *machine, product_surface_speed *out_speed);
    lib_status (*set_speed)(void *machine, product_surface_speed speed);
} app_composed_machine;

#endif
