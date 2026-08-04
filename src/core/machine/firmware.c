#include "type.h"

#include "core/machine/firmware_interface.h"



C_VOID core_machine_firmware_initialize(core_machine_firmware *firmware)
{
    if (firmware != STD_NULL) STD_MEMSET(firmware, 0, sizeof(*firmware));
}

type_status core_machine_firmware_register_service(
    core_machine_firmware *firmware, const core_machine_firmware_service_descriptor *service)
{
    STD_SIZE_T index;
    if (firmware == STD_NULL || service == STD_NULL || service->id == STD_NULL || service->id[0] == '\0' ||
        service->kind < CORE_MACHINE_FIRMWARE_SERVICE_POST || service->kind > CORE_MACHINE_FIRMWARE_SERVICE_INTERRUPT ||
        (service->kind == CORE_MACHINE_FIRMWARE_SERVICE_INTERRUPT && service->vector > 255u)) return TYPE_STATUS_INVALID_ARGUMENT;
    if (firmware->frozen) return TYPE_STATUS_INVALID_STATE;
    for (index = 0u; index < firmware->count; ++index) {
        const core_machine_firmware_service_descriptor *existing = firmware->services[index];
        if (STD_STRCMP(existing->id, service->id) == 0 ||
            (service->kind == CORE_MACHINE_FIRMWARE_SERVICE_INTERRUPT &&
             existing->kind == CORE_MACHINE_FIRMWARE_SERVICE_INTERRUPT && existing->vector == service->vector)) return TYPE_STATUS_UNSUPPORTED;
    }
    if (firmware->count == CORE_MACHINE_FIRMWARE_SERVICE_CAPACITY) return TYPE_STATUS_NO_MEMORY;
    index = firmware->count;
    while (index > 0u && firmware->services[index - 1u]->order > service->order) {
        firmware->services[index] = firmware->services[index - 1u]; --index;
    }
    firmware->services[index] = service; ++firmware->count;
    return TYPE_STATUS_OK;
}

type_status core_machine_firmware_freeze(core_machine_firmware *firmware)
{
    if (firmware == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    firmware->frozen = 1; return TYPE_STATUS_OK;
}

const core_machine_firmware_service_descriptor *core_machine_firmware_service_at(
    const core_machine_firmware *firmware, STD_SIZE_T index)
{
    return firmware == STD_NULL || index >= firmware->count ? STD_NULL : firmware->services[index];
}

const core_machine_firmware_service_descriptor *core_machine_firmware_find_interrupt(
    const core_machine_firmware *firmware, C_UINT vector)
{
    STD_SIZE_T index;

    if (firmware == STD_NULL || vector > 255u) return STD_NULL;
    for (index = 0u; index < firmware->count; ++index) {
        const core_machine_firmware_service_descriptor *service = firmware->services[index];
        if (service->kind == CORE_MACHINE_FIRMWARE_SERVICE_INTERRUPT &&
            service->vector == vector) return service;
    }
    return STD_NULL;
}
