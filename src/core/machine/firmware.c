#include "core/machine/firmware.h"

#include <string.h>

void nxvm_firmware_initialize(nxvm_firmware *firmware)
{
    if (firmware != NULL) memset(firmware, 0, sizeof(*firmware));
}

nxvm_core_status nxvm_firmware_register_service(
    nxvm_firmware *firmware, const nxvm_firmware_service_descriptor *service)
{
    size_t index;
    if (firmware == NULL || service == NULL || service->id == NULL || service->id[0] == '\0' ||
        service->kind < NXVM_FIRMWARE_SERVICE_POST || service->kind > NXVM_FIRMWARE_SERVICE_INTERRUPT ||
        (service->kind == NXVM_FIRMWARE_SERVICE_INTERRUPT && service->vector > 255u)) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    if (firmware->frozen) return NXVM_CORE_STATUS_INVALID_STATE;
    for (index = 0u; index < firmware->count; ++index) {
        const nxvm_firmware_service_descriptor *existing = firmware->services[index];
        if (strcmp(existing->id, service->id) == 0 ||
            (service->kind == NXVM_FIRMWARE_SERVICE_INTERRUPT &&
             existing->kind == NXVM_FIRMWARE_SERVICE_INTERRUPT && existing->vector == service->vector)) return NXVM_CORE_STATUS_UNSUPPORTED;
    }
    if (firmware->count == NXVM_FIRMWARE_SERVICE_CAPACITY) return NXVM_CORE_STATUS_NO_MEMORY;
    index = firmware->count;
    while (index > 0u && firmware->services[index - 1u]->order > service->order) {
        firmware->services[index] = firmware->services[index - 1u]; --index;
    }
    firmware->services[index] = service; ++firmware->count;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_firmware_freeze(nxvm_firmware *firmware)
{
    if (firmware == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    firmware->frozen = 1; return NXVM_CORE_STATUS_OK;
}

const nxvm_firmware_service_descriptor *nxvm_firmware_service_at(
    const nxvm_firmware *firmware, size_t index)
{
    return firmware == NULL || index >= firmware->count ? NULL : firmware->services[index];
}

const nxvm_firmware_service_descriptor *nxvm_firmware_find_interrupt(
    const nxvm_firmware *firmware, unsigned vector)
{
    size_t index;

    if (firmware == NULL || vector > 255u) return NULL;
    for (index = 0u; index < firmware->count; ++index) {
        const nxvm_firmware_service_descriptor *service = firmware->services[index];
        if (service->kind == NXVM_FIRMWARE_SERVICE_INTERRUPT &&
            service->vector == vector) return service;
    }
    return NULL;
}
