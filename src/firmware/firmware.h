#ifndef NXVM_FIRMWARE_H
#define NXVM_FIRMWARE_H

#include <stddef.h>

#include "core/machine/status.h"

#define NXVM_FIRMWARE_SERVICE_CAPACITY 16u

typedef enum nxvm_firmware_service_kind {
    NXVM_FIRMWARE_SERVICE_POST = 1,
    NXVM_FIRMWARE_SERVICE_ROM,
    NXVM_FIRMWARE_SERVICE_INTERRUPT
} nxvm_firmware_service_kind;

typedef struct nxvm_firmware_service_descriptor {
    const char *id;
    nxvm_firmware_service_kind kind;
    unsigned order;
    unsigned vector;
} nxvm_firmware_service_descriptor;

typedef struct nxvm_firmware {
    const nxvm_firmware_service_descriptor *services[NXVM_FIRMWARE_SERVICE_CAPACITY];
    size_t count;
    int frozen;
} nxvm_firmware;

void nxvm_firmware_initialize(nxvm_firmware *firmware);
nxvm_core_status nxvm_firmware_register_service(
    nxvm_firmware *firmware, const nxvm_firmware_service_descriptor *service);
nxvm_core_status nxvm_firmware_freeze(nxvm_firmware *firmware);
const nxvm_firmware_service_descriptor *nxvm_firmware_service_at(
    const nxvm_firmware *firmware, size_t index);
const nxvm_firmware_service_descriptor *nxvm_firmware_find_interrupt(
    const nxvm_firmware *firmware, unsigned vector);

#endif
