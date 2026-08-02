#ifndef NTVDM64_CORE_MACHINE_FIRMWARE_INTERFACE_H
#define NTVDM64_CORE_MACHINE_FIRMWARE_INTERFACE_H

#include <stddef.h>

#include "type.h"

#define CORE_MACHINE_FIRMWARE_SERVICE_CAPACITY 16u

typedef enum core_machine_firmware_service_kind {
    CORE_MACHINE_FIRMWARE_SERVICE_POST = 1,
    CORE_MACHINE_FIRMWARE_SERVICE_ROM,
    CORE_MACHINE_FIRMWARE_SERVICE_INTERRUPT
} core_machine_firmware_service_kind;

typedef struct core_machine_firmware_service_descriptor {
    const char *id;
    core_machine_firmware_service_kind kind;
    unsigned order;
    unsigned vector;
} core_machine_firmware_service_descriptor;

typedef struct core_machine_firmware {
    const core_machine_firmware_service_descriptor *services[CORE_MACHINE_FIRMWARE_SERVICE_CAPACITY];
    size_t count;
    int frozen;
} core_machine_firmware;

void core_machine_firmware_initialize(core_machine_firmware *firmware);
nxvm_core_status core_machine_firmware_register_service(
    core_machine_firmware *firmware, const core_machine_firmware_service_descriptor *service);
nxvm_core_status core_machine_firmware_freeze(core_machine_firmware *firmware);
const core_machine_firmware_service_descriptor *core_machine_firmware_service_at(
    const core_machine_firmware *firmware, size_t index);
const core_machine_firmware_service_descriptor *core_machine_firmware_find_interrupt(
    const core_machine_firmware *firmware, unsigned vector);

#endif
