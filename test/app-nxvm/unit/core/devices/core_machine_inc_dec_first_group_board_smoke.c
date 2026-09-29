#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/debug_interface.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/cpu_board_limit_fixture.h"

typedef struct inc_dec_fault_case {
    lib_u8 code[6];
    lib_u8 bytes;
    lib_bool writable;
    lib_bool out_of_limit;
    lib_u16 memory;
    lib_u32 eax;
    lib_u32 edx;
    lib_u32 flags;
} inc_dec_fault_case;

static lib_i32 inc_dec_first_group_protected_faults(void)
{
    static const inc_dec_fault_case cases[] = {
        {{0xffu,0x06u,0x10u,0u},4u,LIB_TRUE, LIB_TRUE, 0x7fffu,0u,0u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF},
        {{0xffu,0x06u,0x10u,0u},4u,LIB_FALSE,LIB_FALSE,0x7fffu,0u,0u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF},
        {{0xf7u,0x16u,0x10u,0u},4u,LIB_TRUE, LIB_TRUE, 0x55aau,0u,0u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF},
        {{0xf7u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_FALSE,0x55aau,0u,0u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF},
        {{0xf7u,0x1eu,0x10u,0u},4u,LIB_TRUE, LIB_TRUE, 0x7fffu,0u,0u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF},
        {{0xf7u,0x1eu,0x10u,0u},4u,LIB_FALSE,LIB_FALSE,0x7fffu,0u,0u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF},
        {{0xf7u,0x06u,0x10u,0u,0xffu,0xffu},6u,LIB_TRUE,LIB_TRUE,
            0xffffu,0u,0u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF},
        {{0xf7u,0x26u,0x10u,0u},4u,LIB_TRUE,LIB_TRUE,2u,
            0xaabb8000u,0x11223344u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_ZF},
        {{0xf7u,0x2eu,0x10u,0u},4u,LIB_TRUE,LIB_TRUE,2u,
            0xaabb8000u,0x11223344u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_ZF},
        {{0xf7u,0x36u,0x10u,0u},4u,LIB_TRUE,LIB_TRUE,2u,
            5u,0xaabbccddu,VCPU_EFLAGS_CF | VCPU_EFLAGS_OF},
        {{0xf7u,0x3eu,0x10u,0u},4u,LIB_TRUE,LIB_TRUE,2u,
            5u,0xaabbccddu,VCPU_EFLAGS_CF | VCPU_EFLAGS_OF}
    };

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]);
        ++index) {
        const inc_dec_fault_case *entry = &cases[index];
        core_machine *machine = LIB_NULL;
        core_machine_debug_cpu_snapshot after = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        core_machine_run_result result = {0};
        core_machine_debug_register_patch patch = {0};
        lib_u16 observed = 0u;
        lib_i32 failed = !test_cpu_board_limit_prepare(&machine, entry->code,
            entry->bytes, entry->writable, entry->out_of_limit);

        if (!failed) {
            patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
            patch.values[CORE_MACHINE_DEBUG_EAX] = entry->eax;
            patch.values[CORE_MACHINE_DEBUG_EDX] = entry->edx;
            patch.values[CORE_MACHINE_DEBUG_EFLAGS] = entry->flags;
            failed = core_machine_debug_patch_registers(machine, &patch) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x3010u, &entry->memory,
                    sizeof(entry->memory)) != LIB_STATUS_OK ||
                core_machine_run(machine, (core_machine_run_budget){1u,0u},
                    &result) != LIB_STATUS_INTERNAL_ERROR ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                    LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                !diagnostic.first_fault.valid ||
                !(diagnostic.first_fault.exception_mask & VCPUINS_EXCEPT_DF) ||
                core_machine_debug_read_real(machine, 0x0301u, 0u,
                    &observed, sizeof(observed)) != LIB_STATUS_OK ||
                observed != entry->memory || after.eax != entry->eax ||
                after.edx != entry->edx || after.eflags != entry->flags ||
                after.eip != 0u;
        }
        core_machine_destroy(machine);
        if (failed) {
            fprintf(stderr, "S33 protected fault case %u failed\n",
                (unsigned)index);
            return 0;
        }
    }
    return 1;
}

static lib_i32 inc_dec_first_group_divide_delivery(void)
{
    static const lib_u8 code[][3] = {
        {0xf6u,0xf1u}, {0xf6u,0xf9u}, {0xf7u,0xf1u}, {0xf7u,0xf9u},
        {0x66u,0xf7u,0xf1u}, {0x66u,0xf7u,0xf9u}
    };
    static const lib_u8 handler[] = {0xf4u};
    const lib_u16 handler_offset = 0x0100u;
    const lib_u16 code_offset = 0x0200u;
    const lib_u16 handler_segment = 0u;

    for (lib_u8 fault_case = 0u; fault_case != 2u; ++fault_case)
    for (lib_u8 form = 0u; form != sizeof(code) / sizeof(code[0]); ++form) {
        const lib_i32 signed_divide = (form & 1u) != 0u;
        const lib_u8 bytes = form < 2u ? 1u : (form < 4u ? 2u : 4u);
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        lib_u32 eax = fault_case ? (signed_divide ?
            (bytes == 1u ? 0xff80u : (bytes == 2u ?
            0x00008000u : 0x80000000u)) : 0u) : 5u;
        const lib_u32 edx = fault_case ? (bytes == 1u ? 0x11223344u :
            (signed_divide ? (bytes == 2u ? 0x0000ffffu :
            0xffffffffu) : 1u)) : 0xaabbccddu;
        const lib_u32 ecx = fault_case ? (signed_divide ? mask : 1u) : 0u;
        const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        const lib_u8 frame_width = code[form][0] == 0x66u ? 4u : 2u;
        const core_machine_config config = {
            .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
            .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
            .fpu_profile = X86_FPU_PROFILE_NONE
        };
        core_machine *machine = LIB_NULL;
        core_machine_debug_register_patch patch = {0};
        core_machine_debug_cpu_snapshot before = {0}, after = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        core_machine_run_result result = {0};
        lib_u16 frame16[3] = {0u};
        lib_u32 frame32[3] = {0u};
        lib_i32 failed;

        if (fault_case && !signed_divide && bytes == 1u) eax = 0x00000100u;
        failed = core_machine_create(&config, &machine) != LIB_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK;
        if (!failed) {
            patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
            patch.values[CORE_MACHINE_DEBUG_EIP] = code_offset;
            patch.values[CORE_MACHINE_DEBUG_ESP] = 0x8000u;
            patch.values[CORE_MACHINE_DEBUG_EAX] = eax;
            patch.values[CORE_MACHINE_DEBUG_EDX] = edx;
            patch.values[CORE_MACHINE_DEBUG_ECX] = ecx;
            patch.values[CORE_MACHINE_DEBUG_EFLAGS] = flags;
            failed = core_machine_debug_patch_registers(machine, &patch) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(machine, code_offset, code[form],
                    form < 4u ? 2u : 3u) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0u, &handler_offset,
                    sizeof(handler_offset)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 2u, &handler_segment,
                    sizeof(handler_segment)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, handler_offset, handler,
                    sizeof(handler)) != LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK ||
                core_machine_run(machine, (core_machine_run_budget){1u,0u},
                    &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                    LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                diagnostic.first_fault.valid ||
                !diagnostic.last_delivered_exception.valid ||
                !(diagnostic.last_delivered_exception.exception_mask &
                    VCPUINS_EXCEPT_DE) ||
                after.eip != handler_offset || after.eax != eax ||
                after.edx != edx || after.ecx != ecx || after.eflags != flags ||
                after.esp != ((before.esp & 0xffff0000u) |
                    (lib_u16)(before.esp - 3u * frame_width));
            if (!failed && frame_width == 2u)
                failed = core_machine_debug_read_real(machine, 0u,
                    (lib_u16)after.esp, frame16, sizeof(frame16)) !=
                    LIB_STATUS_OK || frame16[0] != code_offset ||
                    frame16[1] != before.cs.selector ||
                    frame16[2] != (lib_u16)((before.eflags &
                        ~VCPU_EFLAGS_RESERVED) | 0x02u);
            if (!failed && frame_width == 4u)
                failed = core_machine_debug_read_real(machine, 0u,
                    (lib_u16)after.esp, frame32, sizeof(frame32)) !=
                    LIB_STATUS_OK || frame32[0] != code_offset ||
                    frame32[1] != before.cs.selector ||
                    frame32[2] != ((before.eflags &
                        ~VCPU_EFLAGS_RESERVED) | 0x02u);
        }
        core_machine_destroy(machine);
        if (failed) {
            fprintf(stderr, "S33 divide delivery case %u/%u failed\n",
                (unsigned)fault_case, (unsigned)form);
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    if (!inc_dec_first_group_protected_faults() ||
        !inc_dec_first_group_divide_delivery()) {
        fputs("M5:T539:S33:BOARD-INC-DEC-GROUP:FAIL\n", stderr);
        return 1;
    }
    puts("M5:T539:S33:BOARD-INC-DEC-GROUP:OK");
    return 0;
}
