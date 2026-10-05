#include "lib/types/types_interface.h"
#include <stdio.h>
#include "x86/core/device_support_interface.h"

#include "x86/core/machine_interface.h"

#define main vm86_lgdt_lidt_s5_vm86_delivery_main
#include "machine_vm86_delivery_smoke.c"
#undef main

static lib_i32 vm86_lgdt_lidt_s5_case(lib_u8 reg)
{
    vm86_delivery_state state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
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
        before = vm86_capture(state.machine);
        failed |= test_core_machine_fixture_run_after_delivery(state.machine, (core_machine_run_budget){ 8u, 0u },
                &result) != LIB_STATUS_OK ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK;
        after = vm86_capture(state.machine);
        if (!failed) failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            diagnostic.first_fault.valid || !diagnostic.last_delivered_exception.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.last_delivered_exception.exception_mask,
                (1u << 13u)) || diagnostic.last_delivered_exception.exception_code != 0u ||
            diagnostic.last_delivered_exception.point.eip != 0u || after.eip != 0x101u ||
            after.esp != VM86_STACK_TOP - 40u ||
            after.eax != before.eax || after.ecx != before.ecx ||
            after.edx != before.edx || after.ebx != before.ebx ||
            after.ebp != before.ebp || after.esi != before.esi ||
            after.edi != before.edi ||
            after.gdtr.base != before.gdtr.base ||
            after.gdtr.limit != before.gdtr.limit ||
            after.idtr.base != before.idtr.base ||
            after.idtr.limit != before.idtr.limit ||
            core_machine_memory_read(state.machine, 0x4400u,
                (void *)observed, sizeof(observed)) != LIB_STATUS_OK ||
            lib_memory_compare(source, observed, sizeof(source)) != 0 ||
            core_machine_memory_read(state.machine,
                VM86_STACK_TOP - 40u, (void *)frame,
                sizeof(frame)) != LIB_STATUS_OK || frame[0] != 0u ||
            frame[1] != 0u || frame[2] != 0x0200u ||
            frame[3] != (CORE_MACHINE_DEBUG_EFLAGS_VM | CORE_MACHINE_DEBUG_EFLAGS_IF) ||
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
