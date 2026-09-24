#ifndef CORE_MACHINE_FIRMWARE_INTERFACE_H
#define CORE_MACHINE_FIRMWARE_INTERFACE_H
#include "lib/types/types_interface.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;
typedef struct core_machine_firmware_context core_machine_firmware_context;

/* Copied real-mode caller state for one firmware-declared software service.
 * Core retains the live CPU state and applies a handled result atomically. */
typedef struct core_machine_firmware_interrupt_frame {
    lib_u16 ax;
    lib_u16 bx;
    lib_u16 cx;
    lib_u16 dx;
    lib_u16 si;
    lib_u16 di;
    lib_u16 bp;
    lib_u16 ds;
    lib_u16 es;
    lib_u16 flags;
} core_machine_firmware_interrupt_frame;

typedef struct core_machine_firmware_interrupt_result {
    lib_u16 ax;
    lib_u16 flags;
} core_machine_firmware_interrupt_result;

typedef lib_status (*core_machine_firmware_software_interrupt)(
    void *provider_context, core_machine_firmware_context *firmware,
    lib_u8 vector, lib_u16 target_segment,
    lib_u16 target_offset,
    const core_machine_firmware_interrupt_frame *input,
    core_machine_firmware_interrupt_result *output, lib_u8 *out_handled);

/* Firmware receives this context only while core synchronously invokes one of
 * its callbacks. It never exposes machine storage or an execution handle. */
typedef struct core_machine_firmware_provider {
    lib_status (*configure)(void *provider_context,
        core_machine_firmware_context *firmware);
    lib_status (*reset)(void *provider_context,
        core_machine_firmware_context *firmware);
    lib_status (*after_run)(void *provider_context,
        core_machine_firmware_context *firmware);
    core_machine_firmware_software_interrupt software_interrupt;
} core_machine_firmware_provider;

lib_status core_machine_bind_firmware_provider(core_machine *machine,
    const core_machine_firmware_provider *provider, void *provider_context);

/* Configuration-only capability. The provider supplies copied ROM bytes;
 * core validates and owns the resulting immutable mapping. */
lib_status core_machine_firmware_register_immutable_rom(
    core_machine_firmware_context *firmware, lib_u32 physical_start,
    const lib_u8 *image, lib_size bytes);

/* Configuration-only alias of a prior immutable ROM mapping. `source_start`
 * and `bytes` select a backing subrange. Core validates that subrange and
 * retains the backing-image lifetime. Earlier providers retain route priority
 * where an alias target overlaps them. */
lib_status core_machine_firmware_register_immutable_rom_alias(
    core_machine_firmware_context *firmware, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes);

/* Runtime whitelist. Every memory and port access remains core-checked and
 * is valid only while its originating callback is active. */
lib_status core_machine_firmware_memory_read(
    core_machine_firmware_context *firmware, lib_u32 physical,
    void *out_data, lib_size size);
lib_status core_machine_firmware_memory_write(
    core_machine_firmware_context *firmware, lib_u32 physical,
    const void *data, lib_size size);
lib_status core_machine_firmware_port_read(
    core_machine_firmware_context *firmware, lib_u16 port,
    lib_u32 *out_value);
lib_status core_machine_firmware_port_write(
    core_machine_firmware_context *firmware, lib_u16 port, lib_u32 value);

#ifdef __cplusplus
}
#endif

#endif
