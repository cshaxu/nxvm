#ifndef NTVDM64_CORE_MACHINE_FIRMWARE_INTERFACE_H
#define NTVDM64_CORE_MACHINE_FIRMWARE_INTERFACE_H


#include "type.h"

#define CORE_MACHINE_FIRMWARE_SERVICE_CAPACITY 16u

typedef enum core_machine_firmware_service_kind {
    CORE_MACHINE_FIRMWARE_SERVICE_POST = 1,
    CORE_MACHINE_FIRMWARE_SERVICE_ROM,
    CORE_MACHINE_FIRMWARE_SERVICE_INTERRUPT
} core_machine_firmware_service_kind;

typedef struct core_machine_firmware_service_descriptor {
    const C_CHAR *id;
    core_machine_firmware_service_kind kind;
    C_UINT order;
    C_UINT vector;
} core_machine_firmware_service_descriptor;

typedef struct core_machine_firmware {
    const core_machine_firmware_service_descriptor *services[CORE_MACHINE_FIRMWARE_SERVICE_CAPACITY];
    STD_SIZE_T count;
    C_INT frozen;
} core_machine_firmware;

C_VOID core_machine_firmware_initialize(core_machine_firmware *firmware);
ntvdm64_status core_machine_firmware_register_service(
    core_machine_firmware *firmware, const core_machine_firmware_service_descriptor *service);
ntvdm64_status core_machine_firmware_freeze(core_machine_firmware *firmware);
const core_machine_firmware_service_descriptor *core_machine_firmware_service_at(
    const core_machine_firmware *firmware, STD_SIZE_T index);
const core_machine_firmware_service_descriptor *core_machine_firmware_find_interrupt(
    const core_machine_firmware *firmware, C_UINT vector);

#endif
