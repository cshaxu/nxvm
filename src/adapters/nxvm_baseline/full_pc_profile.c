#include "adapters/nxvm_baseline/full_pc_profile.h"

#include <string.h>

#include "nxvm-baseline/device/device.h"
#include "product/vm/debug.h"
#include "machine/vm/machine.h"
#include "platform/vm/platform.h"

static int nxvm_baseline_full_pc_active;

static uint16_t nxvm_baseline_read_u16(const void *source)
{
    uint16_t value;

    memcpy(&value, source, sizeof(value));
    return value;
}

nxvm_core_status nxvm_baseline_full_pc_create(
    const nxvm_baseline_full_pc_config *config)
{
    if (config == NULL || nxvm_baseline_full_pc_active ||
        (config->fdd_image == NULL && config->hdd_image == NULL &&
         !config->create_fdd && config->create_hdd_cylinders == 0u)) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    machineInit();
    if ((config->fdd_image != NULL &&
         deviceConnectFloppyInsert(config->fdd_image)) ||
        (config->hdd_image != NULL &&
         deviceConnectHardDiskInsert(config->hdd_image))) {
        machineFinal();
        return NXVM_CORE_STATUS_FAULT;
    }
    if (config->create_fdd) deviceConnectFloppyCreate();
    if (config->create_hdd_cylinders != 0u) {
        deviceConnectHardDiskCreate(config->create_hdd_cylinders);
    }

    deviceConnectBiosSetBoot(config->boot_hdd != 0);
    deviceReset();
    nxvm_baseline_full_pc_active = 1;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_baseline_full_pc_get_reset_vector(
    nxvm_baseline_reset_vector *out_vector)
{
    if (!nxvm_baseline_full_pc_active || out_vector == NULL) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }

    out_vector->cs = nxvm_baseline_read_u16(deviceConnectCpuGetRefCS());
    out_vector->ip = nxvm_baseline_read_u16(deviceConnectCpuGetRefIP());
    return NXVM_CORE_STATUS_OK;
}

void nxvm_baseline_full_pc_run(void)
{
    if (nxvm_baseline_full_pc_active) {
        machineStart();
    }
}

nxvm_core_status nxvm_baseline_full_pc_set_window_display(int enabled)
{
    if (!nxvm_baseline_full_pc_active || device.flagRun) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    platform.flagMode = enabled != 0;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_baseline_full_pc_set_memory_kb(uint32_t kilobytes)
{
    if (!nxvm_baseline_full_pc_active || device.flagRun ||
        kilobytes < 1024u || kilobytes > 16384u) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    deviceConnectRamAllocate((size_t)kilobytes * 1024u);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_baseline_full_pc_reset(void)
{
    if (!nxvm_baseline_full_pc_active || device.flagRun) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    machineReset();
    return NXVM_CORE_STATUS_OK;
}

void nxvm_baseline_full_pc_resume(void)
{
    if (nxvm_baseline_full_pc_active && !device.flagRun) {
        machineResume();
    }
}

nxvm_core_status nxvm_baseline_full_pc_is_running(int *out_running)
{
    if (!nxvm_baseline_full_pc_active || out_running == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    *out_running = device.flagRun != 0;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_baseline_full_pc_debug(void)
{
    if (!nxvm_baseline_full_pc_active || device.flagRun) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    debugMain();
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_baseline_full_pc_remove_fdd(const char *path)
{
    if (!nxvm_baseline_full_pc_active || device.flagRun) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    return deviceConnectFloppyRemove(path) ? NXVM_CORE_STATUS_FAULT : NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_baseline_full_pc_disconnect_hdd(const char *path)
{
    if (!nxvm_baseline_full_pc_active || device.flagRun) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    return deviceConnectHardDiskRemove(path) ? NXVM_CORE_STATUS_FAULT : NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_baseline_full_pc_record_start(const char *path)
{
    if (!nxvm_baseline_full_pc_active || path == NULL || path[0] == '\0') {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    deviceConnectDebugRecordStart(path);
    return NXVM_CORE_STATUS_OK;
}

void nxvm_baseline_full_pc_record_stop(void)
{
    if (nxvm_baseline_full_pc_active) deviceConnectDebugRecordStop();
}

void nxvm_baseline_full_pc_request_stop(void)
{
    if (nxvm_baseline_full_pc_active) {
        deviceStop();
    }
}

void nxvm_baseline_full_pc_destroy(void)
{
    if (nxvm_baseline_full_pc_active) {
        deviceStop();
        machineFinal();
        nxvm_baseline_full_pc_active = 0;
    }
}
