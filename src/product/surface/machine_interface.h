#ifndef PRODUCT_SURFACE_MACHINE_INTERFACE_H
#define PRODUCT_SURFACE_MACHINE_INTERFACE_H

#include "lib/types/types_interface.h"
#include "common/machine/machine_interface.h"

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

/* App composes this opaque machine lifetime before entering Product. Product
 * neither reads configuration nor constructs the private machine: it only
 * creates Common around the complete driver and releases this owned value
 * after Common has stopped. Context and INFO names outlive Product. */
typedef struct app_composed_machine {
    void *machine;
    common_machine_driver driver;
    const void *context;
    lib_status (*bind)(void *machine, common_machine *common);
    void (*destroy)(void *machine);
    /* Optional App capabilities. Composition requires only the machine,
     * driver, bind and destroy values; an App extension may use a capability
     * it provides. */
    lib_status (*information)(const void *context, const void *machine, product_surface_information *out_info);
    lib_status (*get_speed)(const void *machine, product_surface_speed *out_speed);
    lib_status (*set_speed)(void *machine, product_surface_speed speed);
} app_composed_machine;

#endif
