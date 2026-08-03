#include "vm/composition_full_pc.h"

#include <stdlib.h>
#include <string.h>

#include "vm/composition_control.h"
#include "core/machine/cpu.h"
#include "core/machine/memory.h"
#include "vm/machine/debug.h"
#include "core/machine/keyboard_interface.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/profile/default_profile/firmware/bios.h"
#include "vm/platform/vm_request_transport.h"
#include "vm/platform/win32/win32.h"
#include "core/product/debug/debug.h"
#include "vm/composition_machine.h"
#include "vm/composition_debug.h"
#include "vm/platform/platform.h"

struct nxvm_full_pc {
    int active;
    nxvm_vm_request_transport transport;
    vm_composition_live_machine machine;
};

static int nxvm_full_pc_enqueue_keyboard_state(
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
    nxvm_full_pc *full_pc = (nxvm_full_pc *)opaque;

    if (full_pc != NULL && full_pc->active && request != NULL &&
        request->kind == NXVM_PLATFORM_VM_REQUEST_KEYBOARD_STATE) {
        core_machine_keyboard_apply_host_state_to(
            full_pc->machine.keyboard_provider,
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
    const nxvm_full_pc_config *config,
    nxvm_full_pc **out_full_pc)
{
    nxvm_full_pc *full_pc;

    if (config == NULL || out_full_pc == NULL ||
        (config->fdd_image == NULL && config->hdd_image == NULL &&
         !config->create_fdd && config->create_hdd_cylinders == 0u)) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    *out_full_pc = NULL;
    full_pc = (nxvm_full_pc *)calloc(1u, sizeof(*full_pc));
    if (full_pc == NULL) return NXVM_CORE_STATUS_NO_MEMORY;

    machineInit(&full_pc->machine);
    nxvm_vm_request_transport_initialize(&full_pc->transport);
    nxvm_vm_request_transport_bind_consumer(
        &full_pc->transport,
        nxvm_full_pc_consume_request, full_pc);
    vm_platform_run_context_bind_keyboard_state(
        full_pc->machine.platform_run_context,
        nxvm_full_pc_enqueue_keyboard_state, &full_pc->transport);
    vm_composition_control_bind_command_boundary(full_pc->machine.control,
        nxvm_vm_request_transport_observe_execution_boundary,
        &full_pc->transport);
    if ((config->fdd_image != NULL &&
         vm_machine_fdd_insert_for(full_pc->machine.fdd, config->fdd_image)) ||
        (config->hdd_image != NULL &&
         vm_machine_hdd_insert(full_pc->machine.hdd, config->hdd_image))) {
        vm_platform_run_context_bind_keyboard_state(
            full_pc->machine.platform_run_context, NULL, NULL);
        vm_composition_control_bind_command_boundary(full_pc->machine.control, NULL, NULL);
        nxvm_vm_request_transport_close(&full_pc->transport);
        nxvm_vm_request_transport_discard(&full_pc->transport);
        machineFinal(&full_pc->machine);
        free(full_pc);
        return NXVM_CORE_STATUS_FAULT;
    }
    if (config->create_fdd) vm_machine_fdd_create_for(full_pc->machine.fdd);
    if (config->create_hdd_cylinders != 0u) {
        vm_machine_hdd_create(full_pc->machine.hdd, config->create_hdd_cylinders);
    }

    vm_profile_default_bios_set_boot_hdd(full_pc->machine.default_bios,
        config->boot_hdd != 0);
    vm_composition_control_reset(full_pc->machine.control);
    full_pc->active = 1;
    *out_full_pc = full_pc;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_full_pc_get_reset_vector(
    const nxvm_full_pc *full_pc,
    nxvm_vm_reset_vector *out_vector)
{
    if (full_pc == NULL || !full_pc->active || out_vector == NULL) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }

    out_vector->cs = nxvm_baseline_read_u16(
        &full_pc->machine.cpu->data.cs.selector);
    out_vector->ip = nxvm_baseline_read_u16(&full_pc->machine.cpu->data.ip);
    return NXVM_CORE_STATUS_OK;
}

void nxvm_full_pc_run(nxvm_full_pc *full_pc)
{
    if (full_pc != NULL && full_pc->active) {
        machineStart(&full_pc->machine);
    }
}

nxvm_core_status nxvm_full_pc_set_window_display(nxvm_full_pc *full_pc, int enabled)
{
    if (full_pc == NULL || !full_pc->active || vm_composition_control_is_running(full_pc->machine.control)) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    vm_platform_run_context_set_window_display(
        full_pc->machine.platform_run_context, enabled);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_full_pc_set_memory_kb(nxvm_full_pc *full_pc, uint32_t kilobytes)
{
    if (full_pc == NULL || !full_pc->active || vm_composition_control_is_running(full_pc->machine.control) ||
        kilobytes < 1024u || kilobytes > 16384u) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    core_machine_memory_allocate_for(full_pc->machine.ram,
        (size_t)kilobytes * 1024u);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_full_pc_reset(nxvm_full_pc *full_pc)
{
    if (full_pc == NULL || !full_pc->active || vm_composition_control_is_running(full_pc->machine.control)) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    machineReset(&full_pc->machine);
    return NXVM_CORE_STATUS_OK;
}

void nxvm_full_pc_resume(nxvm_full_pc *full_pc)
{
    if (full_pc != NULL && full_pc->active && !vm_composition_control_is_running(full_pc->machine.control)) {
        machineResume(&full_pc->machine);
    }
}

nxvm_core_status nxvm_full_pc_is_running(const nxvm_full_pc *full_pc, int *out_running)
{
    if (full_pc == NULL || !full_pc->active || out_running == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    *out_running = vm_composition_control_is_running(full_pc->machine.control) != 0;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_full_pc_debug(nxvm_full_pc *full_pc)
{
    if (full_pc == NULL || !full_pc->active || vm_composition_control_is_running(full_pc->machine.control)) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    core_product_debug_context_initialize(full_pc->machine.debugger_context);
    debugMain(full_pc->machine.debugger_context,
              vm_composition_debug_target(&full_pc->machine));
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_full_pc_remove_fdd(nxvm_full_pc *full_pc, const char *path)
{
    if (full_pc == NULL || !full_pc->active || vm_composition_control_is_running(full_pc->machine.control)) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    return vm_machine_fdd_remove_for(full_pc->machine.fdd, path) ?
        NXVM_CORE_STATUS_FAULT : NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_full_pc_disconnect_hdd(nxvm_full_pc *full_pc, const char *path)
{
    if (full_pc == NULL || !full_pc->active || vm_composition_control_is_running(full_pc->machine.control)) {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    return vm_machine_hdd_remove(full_pc->machine.hdd, path) ? NXVM_CORE_STATUS_FAULT : NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_full_pc_record_start(nxvm_full_pc *full_pc, const char *path)
{
    if (full_pc == NULL || !full_pc->active || path == NULL || path[0] == '\0') {
        return NXVM_CORE_STATUS_INVALID_STATE;
    }
    vm_machine_debug_record_start(full_pc->machine.debug, path);
    return NXVM_CORE_STATUS_OK;
}

void nxvm_full_pc_record_stop(nxvm_full_pc *full_pc)
{
    if (full_pc != NULL && full_pc->active)
        vm_machine_debug_record_stop(full_pc->machine.debug);
}

void nxvm_full_pc_request_stop(nxvm_full_pc *full_pc)
{
    if (full_pc != NULL && full_pc->active) {
        vm_composition_control_stop(full_pc->machine.control);
    }
}

void nxvm_full_pc_destroy(nxvm_full_pc *full_pc)
{
    if (full_pc != NULL && full_pc->active) {
        vm_platform_run_context_bind_keyboard_state(
            full_pc->machine.platform_run_context, NULL, NULL);
        vm_composition_control_stop(full_pc->machine.control);
        vm_composition_control_bind_command_boundary(full_pc->machine.control, NULL, NULL);
        nxvm_vm_request_transport_close(&full_pc->transport);
        nxvm_vm_request_transport_discard(&full_pc->transport);
        machineFinal(&full_pc->machine);
        full_pc->active = 0;
    }
    free(full_pc);
}
