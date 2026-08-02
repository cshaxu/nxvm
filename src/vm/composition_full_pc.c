#include "vm/composition_full_pc.h"

#include <string.h>

#include "vm/composition_control.h"
#include "core/machine/cpu.h"
#include "core/machine/memory.h"
#include "vm/machine/vdebug.h"
#include "core/machine/keyboard_interface.h"
#include "vm/machine/vfdd.h"
#include "vm/machine/vhdd.h"
#include "vm/profile/default_profile/firmware/vbios.h"
#include "vm/platform/vm_request_transport.h"
#include "vm/platform/win32/win32.h"
#include "core/product/debug/debug.h"
#include "vm/composition_machine.h"
#include "vm/platform/platform.h"

static int nxvm_full_pc_active;
static nxvm_vm_request_transport nxvm_full_pc_transport;

static nxvm_core_status nxvm_full_pc_enqueue_keyboard_state(
    void *opaque, uint32_t asynchronous_keys, uint32_t toggle_keys)
{
    nxvm_platform_vm_request request;

    request.kind = NXVM_PLATFORM_VM_REQUEST_KEYBOARD_STATE;
    request.data.keyboard_state.asynchronous_keys = asynchronous_keys;
    request.data.keyboard_state.toggle_keys = toggle_keys;
    return nxvm_vm_request_transport_enqueue_ingress(
        (nxvm_vm_request_transport *)opaque, &request);
}

static void nxvm_full_pc_consume_request(
    void *opaque, const nxvm_platform_vm_request *request)
{
    (void)opaque;
    if (request != NULL && request->kind == NXVM_PLATFORM_VM_REQUEST_KEYBOARD_STATE) {
        core_machine_keyboard_apply_host_state(
            request->data.keyboard_state.asynchronous_keys,
            request->data.keyboard_state.toggle_keys);
    }
}

static uint16_t nxvm_baseline_read_u16(const void *source)
{
    uint16_t value;

    memcpy(&value, source, sizeof(value));
    return value;
}

nxvm_core_status nxvm_full_pc_create(
    const nxvm_full_pc_config *config)
{
    if (config == NULL || nxvm_full_pc_active ||
        (config->fdd_image == NULL && config->hdd_image == NULL &&
         !config->create_fdd && config->create_hdd_cylinders == 0u)) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    machineInit();
    nxvm_vm_request_transport_initialize(&nxvm_full_pc_transport);
    nxvm_vm_request_transport_bind_consumer(
        &nxvm_full_pc_transport,
        nxvm_full_pc_consume_request, NULL);
    win32KeyboardBindStateSink(nxvm_full_pc_enqueue_keyboard_state,
                               &nxvm_full_pc_transport);
    vm_composition_control_bind_command_boundary(
        nxvm_vm_request_transport_observe_execution_boundary,
        &nxvm_full_pc_transport);
    if ((config->fdd_image != NULL &&
         vm_machine_fdd_insert(config->fdd_image)) ||
        (config->hdd_image != NULL &&
         vm_machine_hdd_insert(config->hdd_image))) {
        win32KeyboardBindStateSink(NULL, NULL);
        vm_composition_control_bind_command_boundary(NULL, NULL);
        nxvm_vm_request_transport_close(&nxvm_full_pc_transport);
        nxvm_vm_request_transport_discard(&nxvm_full_pc_transport);
        machineFinal();
        return NXVM_CORE_STATUS_FAULT;
    }
    if (config->create_fdd) vm_machine_fdd_create();
    if (config->create_hdd_cylinders != 0u) {
        vm_machine_hdd_create(config->create_hdd_cylinders);
    }

    vm_profile_default_bios_set_boot_hdd(config->boot_hdd != 0);
    vm_composition_control_reset();
    nxvm_full_pc_active = 1;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_full_pc_get_reset_vector(
    nxvm_vm_reset_vector *out_vector)
{
    if (!nxvm_full_pc_active || out_vector == NULL) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }

    out_vector->cs = nxvm_baseline_read_u16(&vcpu.data.cs.selector);
    out_vector->ip = nxvm_baseline_read_u16(&vcpu.data.ip);
    return NXVM_CORE_STATUS_OK;
}

void nxvm_full_pc_run(void)
{
    if (nxvm_full_pc_active) {
        machineStart();
    }
}

nxvm_core_status nxvm_full_pc_set_window_display(int enabled)
{
    if (!nxvm_full_pc_active || vm_composition_control_is_running()) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    platform.flagMode = enabled != 0;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_full_pc_set_memory_kb(uint32_t kilobytes)
{
    if (!nxvm_full_pc_active || vm_composition_control_is_running() ||
        kilobytes < 1024u || kilobytes > 16384u) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    core_machine_memory_allocate((size_t)kilobytes * 1024u);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_full_pc_reset(void)
{
    if (!nxvm_full_pc_active || vm_composition_control_is_running()) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    machineReset();
    return NXVM_CORE_STATUS_OK;
}

void nxvm_full_pc_resume(void)
{
    if (nxvm_full_pc_active && !vm_composition_control_is_running()) {
        machineResume();
    }
}

nxvm_core_status nxvm_full_pc_is_running(int *out_running)
{
    if (!nxvm_full_pc_active || out_running == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    *out_running = vm_composition_control_is_running() != 0;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_full_pc_debug(void)
{
    if (!nxvm_full_pc_active || vm_composition_control_is_running()) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    debugMain();
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_full_pc_remove_fdd(const char *path)
{
    if (!nxvm_full_pc_active || vm_composition_control_is_running()) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    return vm_machine_fdd_remove(path) ? NXVM_CORE_STATUS_FAULT : NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_full_pc_disconnect_hdd(const char *path)
{
    if (!nxvm_full_pc_active || vm_composition_control_is_running()) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    return vm_machine_hdd_remove(path) ? NXVM_CORE_STATUS_FAULT : NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_full_pc_record_start(const char *path)
{
    if (!nxvm_full_pc_active || path == NULL || path[0] == '\0') {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    vm_machine_debug_record_start(path);
    return NXVM_CORE_STATUS_OK;
}

void nxvm_full_pc_record_stop(void)
{
    if (nxvm_full_pc_active) vm_machine_debug_record_stop();
}

void nxvm_full_pc_request_stop(void)
{
    if (nxvm_full_pc_active) {
        vm_composition_control_stop();
    }
}

void nxvm_full_pc_destroy(void)
{
    if (nxvm_full_pc_active) {
        win32KeyboardBindStateSink(NULL, NULL);
        vm_composition_control_stop();
        vm_composition_control_bind_command_boundary(NULL, NULL);
        nxvm_vm_request_transport_close(&nxvm_full_pc_transport);
        nxvm_vm_request_transport_discard(&nxvm_full_pc_transport);
        machineFinal();
        nxvm_full_pc_active = 0;
    }
}
