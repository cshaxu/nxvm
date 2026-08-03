#include "type.h"

#include "vm/composition_full_pc.h"





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

struct vm_composition_full_pc {
    C_INT active;
    vm_platform_request_transport transport;
    vm_composition_live_machine machine;
};

static C_INT vm_composition_full_pc_enqueue_keyboard_state(
    C_VOID *opaque, uint32_t asynchronous_keys, uint32_t toggle_keys)
{
    nxvm_platform_vm_request request;

    request.kind = NXVM_PLATFORM_VM_REQUEST_KEYBOARD_STATE;
    request.data.keyboard_state.asynchronous_keys = asynchronous_keys;
    request.data.keyboard_state.toggle_keys = toggle_keys;
    return vm_platform_request_transport_enqueue_ingress(
        (vm_platform_request_transport *)opaque, &request);
}

static C_VOID vm_composition_full_pc_consume_request(
    C_VOID *opaque, const nxvm_platform_vm_request *request)
{
    vm_composition_full_pc *full_pc = (vm_composition_full_pc *)opaque;

    if (full_pc != STD_NULL && full_pc->active && request != STD_NULL &&
        request->kind == NXVM_PLATFORM_VM_REQUEST_KEYBOARD_STATE) {
        core_machine_keyboard_apply_host_state_to(
            full_pc->machine.keyboard_provider,
            request->data.keyboard_state.asynchronous_keys,
            request->data.keyboard_state.toggle_keys);
    }
}

static uint16_t nxvm_baseline_read_u16(const C_VOID *source)
{
    uint16_t value;

    STD_MEMCPY(&value, source, sizeof(value));
    return value;
}

ntvdm64_status vm_composition_full_pc_create(
    const vm_composition_full_pc_config *config,
    vm_composition_full_pc **out_full_pc)
{
    vm_composition_full_pc *full_pc;

    if (config == STD_NULL || out_full_pc == STD_NULL ||
        (config->fdd_image == STD_NULL && config->hdd_image == STD_NULL &&
         !config->create_fdd && config->create_hdd_cylinders == 0u)) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    *out_full_pc = STD_NULL;
    full_pc = (vm_composition_full_pc *)STD_CALLOC(1u, sizeof(*full_pc));
    if (full_pc == STD_NULL) return NTVDM64_STATUS_NO_MEMORY;

    vm_composition_initialize(&full_pc->machine);
    vm_platform_request_transport_initialize(&full_pc->transport);
    vm_platform_request_transport_bind_consumer(
        &full_pc->transport,
        vm_composition_full_pc_consume_request, full_pc);
    vm_platform_run_context_bind_keyboard_state(
        full_pc->machine.platform_run_context,
        vm_composition_full_pc_enqueue_keyboard_state, &full_pc->transport);
    vm_composition_control_bind_command_boundary(full_pc->machine.control,
        vm_platform_request_transport_observe_execution_boundary,
        &full_pc->transport);
    if ((config->fdd_image != STD_NULL &&
         vm_machine_fdd_insert_for(full_pc->machine.fdd, config->fdd_image)) ||
        (config->hdd_image != STD_NULL &&
         vm_machine_hdd_insert(full_pc->machine.hdd, config->hdd_image))) {
        vm_platform_run_context_bind_keyboard_state(
            full_pc->machine.platform_run_context, STD_NULL, STD_NULL);
        vm_composition_control_bind_command_boundary(full_pc->machine.control, STD_NULL, STD_NULL);
        vm_platform_request_transport_close(&full_pc->transport);
        vm_platform_request_transport_discard(&full_pc->transport);
        vm_composition_finalize(&full_pc->machine);
        STD_FREE(full_pc);
        return NTVDM64_STATUS_FAULT;
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
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_composition_full_pc_get_reset_vector(
    const vm_composition_full_pc *full_pc,
    vm_composition_reset_vector *out_vector)
{
    if (full_pc == STD_NULL || !full_pc->active || out_vector == STD_NULL) {
        return NTVDM64_STATUS_INVALID_STATE;
    }

    out_vector->cs = nxvm_baseline_read_u16(
        &full_pc->machine.cpu->data.cs.selector);
    out_vector->ip = nxvm_baseline_read_u16(&full_pc->machine.cpu->data.ip);
    return NTVDM64_STATUS_OK;
}

C_VOID vm_composition_full_pc_run(vm_composition_full_pc *full_pc)
{
    if (full_pc != STD_NULL && full_pc->active) {
        vm_composition_start(&full_pc->machine);
    }
}

ntvdm64_status vm_composition_full_pc_set_window_display(vm_composition_full_pc *full_pc, C_INT enabled)
{
    if (full_pc == STD_NULL || !full_pc->active || vm_composition_control_is_running(full_pc->machine.control)) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    vm_platform_run_context_set_window_display(
        full_pc->machine.platform_run_context, enabled);
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_composition_full_pc_set_memory_kb(vm_composition_full_pc *full_pc, uint32_t kilobytes)
{
    if (full_pc == STD_NULL || !full_pc->active || vm_composition_control_is_running(full_pc->machine.control) ||
        kilobytes < 1024u || kilobytes > 16384u) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    core_machine_memory_allocate_for(full_pc->machine.ram,
        (STD_SIZE_T)kilobytes * 1024u);
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_composition_full_pc_reset(vm_composition_full_pc *full_pc)
{
    if (full_pc == STD_NULL || !full_pc->active || vm_composition_control_is_running(full_pc->machine.control)) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    vm_composition_reset(&full_pc->machine);
    return NTVDM64_STATUS_OK;
}

C_VOID vm_composition_full_pc_resume(vm_composition_full_pc *full_pc)
{
    if (full_pc != STD_NULL && full_pc->active && !vm_composition_control_is_running(full_pc->machine.control)) {
        vm_composition_resume(&full_pc->machine);
    }
}

ntvdm64_status vm_composition_full_pc_is_running(const vm_composition_full_pc *full_pc, C_INT *out_running)
{
    if (full_pc == STD_NULL || !full_pc->active || out_running == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    *out_running = vm_composition_control_is_running(full_pc->machine.control) != 0;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_composition_full_pc_debug(vm_composition_full_pc *full_pc)
{
    if (full_pc == STD_NULL || !full_pc->active || vm_composition_control_is_running(full_pc->machine.control)) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    core_product_debug_context_initialize(full_pc->machine.debugger_context);
    core_product_debug_main(full_pc->machine.debugger_context,
              vm_composition_debug_target(&full_pc->machine));
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_composition_full_pc_remove_fdd(vm_composition_full_pc *full_pc, const C_CHAR *path)
{
    if (full_pc == STD_NULL || !full_pc->active || vm_composition_control_is_running(full_pc->machine.control)) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    return vm_machine_fdd_remove_for(full_pc->machine.fdd, path) ?
        NTVDM64_STATUS_FAULT : NTVDM64_STATUS_OK;
}

ntvdm64_status vm_composition_full_pc_disconnect_hdd(vm_composition_full_pc *full_pc, const C_CHAR *path)
{
    if (full_pc == STD_NULL || !full_pc->active || vm_composition_control_is_running(full_pc->machine.control)) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    return vm_machine_hdd_remove(full_pc->machine.hdd, path) ? NTVDM64_STATUS_FAULT : NTVDM64_STATUS_OK;
}

ntvdm64_status vm_composition_full_pc_record_start(vm_composition_full_pc *full_pc, const C_CHAR *path)
{
    if (full_pc == STD_NULL || !full_pc->active || path == STD_NULL || path[0] == '\0') {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    vm_machine_debug_record_start(full_pc->machine.debug, path);
    return NTVDM64_STATUS_OK;
}

C_VOID vm_composition_full_pc_record_stop(vm_composition_full_pc *full_pc)
{
    if (full_pc != STD_NULL && full_pc->active)
        vm_machine_debug_record_stop(full_pc->machine.debug);
}

C_VOID vm_composition_full_pc_request_stop(vm_composition_full_pc *full_pc)
{
    if (full_pc != STD_NULL && full_pc->active) {
        vm_composition_control_stop(full_pc->machine.control);
    }
}

C_VOID vm_composition_full_pc_destroy(vm_composition_full_pc *full_pc)
{
    if (full_pc != STD_NULL && full_pc->active) {
        vm_platform_run_context_bind_keyboard_state(
            full_pc->machine.platform_run_context, STD_NULL, STD_NULL);
        vm_composition_control_stop(full_pc->machine.control);
        vm_composition_control_bind_command_boundary(full_pc->machine.control, STD_NULL, STD_NULL);
        vm_platform_request_transport_close(&full_pc->transport);
        vm_platform_request_transport_discard(&full_pc->transport);
        vm_composition_finalize(&full_pc->machine);
        full_pc->active = 0;
    }
    STD_FREE(full_pc);
}
