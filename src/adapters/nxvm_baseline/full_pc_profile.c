#include "adapters/nxvm_baseline/full_pc_profile.h"

#include <string.h>

#include "nxvm-baseline/device/device.h"
#include "nxvm-baseline/machine.h"

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
        (config->fdd_image == NULL && config->hdd_image == NULL)) {
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
        deviceStart();
    }
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
