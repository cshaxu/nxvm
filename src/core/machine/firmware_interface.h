#ifndef CORE_MACHINE_FIRMWARE_INTERFACE_H
#define CORE_MACHINE_FIRMWARE_INTERFACE_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;
typedef struct core_machine_firmware_context core_machine_firmware_context;

/* Copied real-mode caller state for one firmware-declared software service.
 * Core retains the live CPU state and applies a handled result atomically. */
typedef struct core_machine_firmware_interrupt_frame {
    type_unsigned_16 ax;
    type_unsigned_16 bx;
    type_unsigned_16 cx;
    type_unsigned_16 dx;
    type_unsigned_16 si;
    type_unsigned_16 di;
    type_unsigned_16 bp;
    type_unsigned_16 ds;
    type_unsigned_16 es;
    type_unsigned_16 flags;
} core_machine_firmware_interrupt_frame;

typedef struct core_machine_firmware_interrupt_result {
    type_unsigned_16 ax;
    type_unsigned_16 flags;
} core_machine_firmware_interrupt_result;

typedef type_status (*core_machine_firmware_software_interrupt)(
    C_VOID *provider_context, core_machine_firmware_context *firmware,
    type_unsigned_8 vector, type_unsigned_16 target_segment,
    type_unsigned_16 target_offset,
    const core_machine_firmware_interrupt_frame *input,
    core_machine_firmware_interrupt_result *output, type_bool *out_handled);

/* Firmware receives this context only while core synchronously invokes one of
 * its callbacks. It never exposes machine storage or an execution handle. */
typedef struct core_machine_firmware_provider {
    type_status (*configure)(C_VOID *provider_context,
        core_machine_firmware_context *firmware);
    type_status (*reset)(C_VOID *provider_context,
        core_machine_firmware_context *firmware);
    type_status (*after_run)(C_VOID *provider_context,
        core_machine_firmware_context *firmware);
    core_machine_firmware_software_interrupt software_interrupt;
} core_machine_firmware_provider;

type_status core_machine_bind_firmware_provider(core_machine *machine,
    const core_machine_firmware_provider *provider, C_VOID *provider_context);

/* Configuration-only capability. The provider supplies copied ROM bytes;
 * core validates and owns the resulting immutable mapping. */
type_status core_machine_firmware_register_immutable_rom(
    core_machine_firmware_context *firmware, type_unsigned_32 physical_start,
    const type_unsigned_8 *image, STD_SIZE_T bytes);

/* Configuration-only alias of a prior immutable ROM mapping. `source_start`
 * and `bytes` select a backing subrange. Core validates that subrange and
 * retains the backing-image lifetime. Earlier providers retain route priority
 * where an alias target overlaps them. */
type_status core_machine_firmware_register_immutable_rom_alias(
    core_machine_firmware_context *firmware, type_unsigned_32 source_start,
    type_unsigned_32 physical_start, STD_SIZE_T bytes);

/* Runtime whitelist. Every memory and port access remains core-checked and
 * is valid only while its originating callback is active. */
type_status core_machine_firmware_memory_read(
    core_machine_firmware_context *firmware, type_unsigned_32 physical,
    C_VOID *out_data, STD_SIZE_T size);
type_status core_machine_firmware_memory_write(
    core_machine_firmware_context *firmware, type_unsigned_32 physical,
    const C_VOID *data, STD_SIZE_T size);
/* AH=87h is the only current caller: it has the documented IBM AT sequence
 * of enabling A20 for its protected-mode transfer and disabling it on exit. */
type_status core_machine_firmware_set_a20(
    core_machine_firmware_context *firmware, type_bool enabled);
type_status core_machine_firmware_port_read(
    core_machine_firmware_context *firmware, type_unsigned_16 port,
    type_unsigned_32 *out_value);
type_status core_machine_firmware_port_write(
    core_machine_firmware_context *firmware, type_unsigned_16 port, type_unsigned_32 value);

/* The sole admitted restricted state operation: firmware may factually ask
 * core to end the current product run after it has consumed its own report. */
type_status core_machine_firmware_request_stop(
    core_machine_firmware_context *firmware);

#ifdef __cplusplus
}
#endif

#endif
