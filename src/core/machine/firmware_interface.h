#ifndef CORE_MACHINE_FIRMWARE_INTERFACE_H
#define CORE_MACHINE_FIRMWARE_INTERFACE_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;
typedef struct core_machine_firmware_context core_machine_firmware_context;

/* Firmware receives this context only while core synchronously invokes one of
 * its callbacks. It never exposes machine storage or an execution handle. */
typedef struct core_machine_firmware_provider {
    type_status (*configure)(C_VOID *provider_context,
        core_machine_firmware_context *firmware);
    type_status (*reset)(C_VOID *provider_context,
        core_machine_firmware_context *firmware);
    type_status (*after_run)(C_VOID *provider_context,
        core_machine_firmware_context *firmware);
} core_machine_firmware_provider;

type_status core_machine_bind_firmware_provider(core_machine *machine,
    const core_machine_firmware_provider *provider, C_VOID *provider_context);

/* Configuration-only capability. The provider supplies copied ROM bytes;
 * core validates and owns the resulting immutable mapping. */
type_status core_machine_firmware_register_immutable_rom(
    core_machine_firmware_context *firmware, type_unsigned_32 physical_start,
    const type_unsigned_8 *image, STD_SIZE_T bytes);

/* Runtime whitelist. Every memory and port access remains core-checked and
 * is valid only while its originating callback is active. */
type_status core_machine_firmware_memory_read(
    core_machine_firmware_context *firmware, type_unsigned_32 physical,
    C_VOID *out_data, STD_SIZE_T size);
type_status core_machine_firmware_memory_write(
    core_machine_firmware_context *firmware, type_unsigned_32 physical,
    const C_VOID *data, STD_SIZE_T size);
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
