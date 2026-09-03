#include "type.h"

#include "core/machine/entry_plan_interface.h"
#include "core/machine/machine_interface.h"
#include "core/platform/input_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/platform/vm_request_transport.h"
#include "../support/rom/session_assets.h"

#define VM_KBC_AUX_BOOT_BUDGET 500000u
#define VM_KBC_AUX_COUNT_ADDRESS 0x0500u
#define VM_KBC_AUX_BYTES_ADDRESS 0x0510u

static const type_unsigned_8 vm_kbc_aux_boot_code[] = {
    0xfau, 0x31u, 0xc0u, 0x8eu, 0xd8u,
    0xc7u, 0x06u, 0xd0u, 0x01u, 0x80u, 0x7cu,
    0xc7u, 0x06u, 0xd2u, 0x01u, 0x00u, 0x00u,
    /* PIC ICWs: master 08h, slave 70h, cascade IRQ2.  A unit entry plan
     * intentionally bypasses ROM POST, so it establishes this guest state. */
    0xb0u, 0x11u, 0xe6u, 0x20u, 0xe6u, 0xa0u,
    0xb0u, 0x08u, 0xe6u, 0x21u,
    0xb0u, 0x70u, 0xe6u, 0xa1u,
    0xb0u, 0x04u, 0xe6u, 0x21u,
    0xb0u, 0x02u, 0xe6u, 0xa1u,
    0xb0u, 0x01u, 0xe6u, 0x21u, 0xe6u, 0xa1u,
    0xb0u, 0xfbu, 0xe6u, 0x21u,
    0xb0u, 0xefu, 0xe6u, 0xa1u,
    0xfbu, 0xb0u, 0xd4u, 0xe6u, 0x64u,
    0xb0u, 0xf4u, 0xe6u, 0x60u,
    0xf4u, 0xebu, 0xfdu
};

static const type_unsigned_8 vm_kbc_aux_irq12_handler[] = {
    0x50u, 0x53u, 0xe4u, 0x60u, 0x8bu, 0x1eu, 0x00u, 0x05u,
    0x88u, 0x87u, 0x10u, 0x05u, 0x43u, 0x89u, 0x1eu, 0x00u, 0x05u,
    0xb0u, 0x20u, 0xe6u, 0xa0u, 0xe6u, 0x20u, 0x5bu, 0x58u, 0xcfu
};

static C_INT vm_kbc_aux_read_count(vm_session *session, type_unsigned_16 *out_count)
{
    return core_machine_memory_read(session->core_machine,
        VM_KBC_AUX_COUNT_ADDRESS, out_count, sizeof(*out_count)) == TYPE_STATUS_OK;
}

static C_INT vm_kbc_aux_run_until_count(vm_session *session, type_unsigned_16 expected)
{
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    type_unsigned_32 instruction;
    type_unsigned_16 count = 0u;

    for (instruction = 0u; instruction < VM_KBC_AUX_BOOT_BUDGET; ++instruction) {
        if (core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT) return 0;
        if (vm_kbc_aux_read_count(session, &count) && count >= expected) return 1;
    }
    return 0;
}

C_INT main(C_VOID)
{
    const vm_session_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    const core_machine_entry_plan_preload preloads[] = {
        { 0x7c00u, vm_kbc_aux_boot_code, sizeof(vm_kbc_aux_boot_code) },
        { 0x7c80u, vm_kbc_aux_irq12_handler, sizeof(vm_kbc_aux_irq12_handler) }
    };
    const core_machine_entry_plan plan = {
        .state = {.ip = 0x7c00u, .sp = 0x7000u, .eflags = 0x0200u},
        .entry_physical = 0x7c00u,
        .entry_route = CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM,
        .preloads = preloads,
        .preload_count = sizeof(preloads) / sizeof(preloads[0])
    };
    vm_session *session = STD_NULL;
    type_unsigned_8 bytes[4] = {0};
    type_unsigned_16 count = 0u;
    C_INT passed = 0;
    C_INT stage = 0;
    type_status create_status;
    type_status plan_status = TYPE_STATUS_OK;

    create_status = vm_test_default_pc_at_session_create(&config, &session);
    if (create_status != TYPE_STATUS_OK || session == STD_NULL ||
        (plan_status = core_machine_apply_entry_plan(session->core_machine,
            &plan)) != TYPE_STATUS_OK) { stage = 1; goto done; }
    if (!vm_kbc_aux_run_until_count(session, 1u) ||
        !vm_kbc_aux_read_count(session, &count) || count != 1u ||
        core_machine_memory_read(session->core_machine, VM_KBC_AUX_BYTES_ADDRESS,
            bytes, 1u) != TYPE_STATUS_OK || bytes[0] != 0xfau) { stage = 2; goto done; }
    {
        core_platform_input_event event = {0};

        event.kind = CORE_PLATFORM_INPUT_RELATIVE_MOUSE;
        event.data.relative_mouse.delta_x = 5;
        event.data.relative_mouse.delta_y = 3;
        event.data.relative_mouse.buttons = 0x01u;
        if (vm_session_submit_host_input(session, &event) != TYPE_STATUS_OK) { stage = 3; goto done; }
    }
    if (!vm_kbc_aux_read_count(session, &count) || count != 1u) goto done;
    vm_platform_request_transport_observe_execution_boundary(session->request_transport);
    if (!vm_kbc_aux_run_until_count(session, 4u) ||
        core_machine_memory_read(session->core_machine, VM_KBC_AUX_BYTES_ADDRESS,
            bytes, sizeof(bytes)) != TYPE_STATUS_OK || bytes[0] != 0xfau ||
        bytes[1] != 0x29u || bytes[2] != 0x05u || bytes[3] != 0xfdu) { stage = 4; goto done; }
    passed = 1;
done:
    vm_session_destroy(session);
    if (!passed) {
        STD_FPRINTF(STD_STDERR, "M5:T515:S3:KBC-AUX-UNIT:stage=%d create=%d plan=%d count=%u bytes=%02X/%02X/%02X/%02X\n",
            stage, create_status, plan_status, count, bytes[0], bytes[1], bytes[2], bytes[3]);
        return 1;
    }
    STD_PRINTF("M5:T229:S3:AUX:GUEST:OK\n");
    return 0;
}
