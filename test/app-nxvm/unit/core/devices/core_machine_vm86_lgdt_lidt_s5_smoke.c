#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/machine_interface.h"

#define main vm86_lgdt_lidt_s5_vm86_delivery_main
#include "core_machine_vm86_delivery_smoke.c"
#undef main

static lib_i32 vm86_lgdt_lidt_s5_case(lib_u8 reg)
{
    vm86_delivery_state state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_u32 frame[10u] = { 0u };
    lib_u8 source[6u] = { 0x5au, 0x5au, 0x5au, 0x5au, 0x5au, 0x5au };
    lib_u8 observed[6u] = { 0u };
    lib_u8 code[] = { 0x0fu, 0x01u, (lib_u8)(0x16u | (reg << 3u)),
        0x00u, 0x04u };
    lib_i32 failed = !vm86_delivery_prepare(&state, 13u);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0x2000u, code,
                sizeof(code)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x4400u, source,
                sizeof(source)) != LIB_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){ 8u, 0u },
                &result) != LIB_STATUS_OK ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            diagnostic.first_fault.valid || !diagnostic.last_delivered_exception.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_GP) || diagnostic.last_delivered_exception.exception_code != 0u ||
            diagnostic.last_delivered_exception.point.eip != 0u || after.data.eip != 0x101u ||
            after.data.esp != VM86_STACK_TOP - 40u ||
            after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi ||
            after.data.gdtr.base != before.data.gdtr.base ||
            after.data.gdtr.limit != before.data.gdtr.limit ||
            after.data.idtr.base != before.data.idtr.base ||
            after.data.idtr.limit != before.data.idtr.limit ||
            core_machine_memory_read_physical(&state.machine->executor_memory, 0x4400u,
                (lib_uptr)observed, sizeof(observed)) != LIB_STATUS_OK ||
            lib_memory_compare(source, observed, sizeof(source)) != 0 ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                VM86_STACK_TOP - 40u, (lib_uptr)frame,
                sizeof(frame)) != LIB_STATUS_OK || frame[0] != 0u ||
            frame[1] != 0u || frame[2] != 0x0200u ||
            frame[3] != (VCPU_EFLAGS_VM | VCPU_EFLAGS_IF) ||
            frame[4] != 0x1234u || frame[5] != 0x0300u ||
            frame[6] != 0x0500u || frame[7] != 0x0400u ||
            frame[8] != 0x0600u || frame[9] != 0x0700u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!vm86_lgdt_lidt_s5_case(2u) || !vm86_lgdt_lidt_s5_case(3u)) return 1;
    printf("M5:T321:S5:VM86-LGDT-LIDT:OK\n");
    return 0;
}
