#include "support/cpu_instruction_fixture.h"
#include "core/chips/cpu/cpu_timing.h"
#include "lib/types/file.h"

_Static_assert(CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_COMPATIBILITY == 11,
    "Existing timing-origin values must remain stable");

typedef enum transfer_form {
    TRANSFER_NEAR_JMP,
    TRANSFER_FAR_JMP,
    TRANSFER_GATE_JMP,
    TRANSFER_NEAR_JCC,
    TRANSFER_SHORT_JCC,
    TRANSFER_INT16,
    TRANSFER_TRAP16,
    TRANSFER_INT32,
    TRANSFER_TRAP32,
    TRANSFER_INT3,
    TRANSFER_INTO,
    TRANSFER_GATE_CALL,
    TRANSFER_SAME_IRET,
    TRANSFER_SAME_RET,
    TRANSFER_OUTER_IRET,
    TRANSFER_OUTER_RET,
    TRANSFER_TASK16_TO16,
    TRANSFER_TASK32_TO16,
    TRANSFER_TASK16_TO32,
    TRANSFER_TASK32_TO32,
    TRANSFER_FORM_COUNT,
} transfer_form;

static void transfer_prepare(cpu_instruction_fixture *state, lib_bool code32)
{
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0x40u,0,
        0,0x20u,8u,0,0,0x8cu,0,0
    };
    const lib_u32 directory = 0x8003u;
    const lib_u32 pages[] = {0x0003u,0x9003u,0u,0xa003u};
    const lib_u8 page_fault_gate[] = {0,2u,8u,0,0,0x8eu,0,0};

    cpu_instruction_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
    gdt[14] = code32 ? 0x40u : 0u;
    lib_memory_copy(state->memory + 0x300u, gdt, sizeof(gdt));
    lib_memory_copy(state->memory + 0x470u, page_fault_gate, sizeof(page_fault_gate));
    lib_memory_copy(state->memory + 0x7000u, &directory, sizeof(directory));
    lib_memory_copy(state->memory + 0x8000u, pages, sizeof(pages));
    state->memory[0x200u] = 0x90u;
    state->cpu.data.gdtr.base = 0x300u;
    state->cpu.data.gdtr.limit = sizeof(gdt) - 1u;
    state->cpu.data.gdtr.flagValid = LIB_TRUE;
    state->cpu.data.idtr.base = 0x400u;
    state->cpu.data.idtr.limit = 0x7fu;
    state->cpu.data.idtr.flagValid = LIB_TRUE;
    state->cpu.data.cs.selector = 8u;
    state->cpu.data.cs.seg.exec.defsize = code32;
    state->cpu.data.cs.limit = 0xffffu;
    state->cpu.data.ss.selector = 16u;
    state->cpu.data.ss.seg.data.big = LIB_TRUE;
    state->cpu.data.esp = 0x3100u;
    state->cpu.data.eip = 0x1ff0u;
    state->cpu.data.cr3 = 0x7000u;
    state->cpu.data.cr0 |= VCPU_CR0_PE | VCPU_CR0_PG;
}

static lib_i32 transfer_unmapped_target(void)
{
    static cpu_instruction_fixture state;
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32) {
        for (lib_u8 form = TRANSFER_NEAR_JMP; form < TRANSFER_FORM_COUNT; ++form) {
            lib_u64 estimated_ticks = 0u;
            for (lib_u8 mapping = 0u; mapping < 2u; ++mapping) {
                const lib_bool mapped = mapping != 0u;
                lib_u8 code[7] = {0};
                lib_u8 bytes = code32 ? 5u : 3u;
                const lib_u32 target = 0x2000u;
                const lib_bool outer_return = form == TRANSFER_OUTER_IRET || form == TRANSFER_OUTER_RET;
                const lib_bool fixed_interrupt = form >= TRANSFER_INT16 &&
                    form <= TRANSFER_INTO;
                const lib_bool fixed_timing = fixed_interrupt ||
                    form == TRANSFER_SAME_IRET || outer_return ||
                    form >= TRANSFER_TASK16_TO16;
                static const lib_u16 task_clocks[] = {285u,285u,310u,392u};
                const lib_u16 target_selector = outer_return ? 35u : 8u;
                lib_u32 operand;
                lib_u32 target_pte;

                transfer_prepare(&state, code32 ? LIB_TRUE : LIB_FALSE);
                if (form == TRANSFER_GATE_CALL && !code32) state.memory[0x31du] = 0x84u;
                if (form >= TRANSFER_TASK16_TO16) {
                    const lib_bool old32 = ((form - TRANSFER_TASK16_TO16) & 1u) != 0u;
                    const lib_bool new32 = ((form - TRANSFER_TASK16_TO16) & 2u) != 0u;
                    const lib_u8 old_descriptor[] = {
                        old32 ? 0x67u : 0x2bu,0,0,6u,0,old32 ? 0x8bu : 0x83u,0,0
                    };
                    const lib_u8 new_descriptor[] = {
                        new32 ? 0x67u : 0x2bu,0,0,0x0cu,0,new32 ? 0x89u : 0x81u,0,0
                    };
                    const lib_u32 directory = 0x7000u;
                    const lib_u32 flags = 2u;
                    const lib_u32 stack = 0x3100u;
                    const lib_u16 code_selector = 8u;
                    const lib_u16 data_selector = 16u;

                    lib_memory_copy(state.memory + 0x320u, old_descriptor, sizeof(old_descriptor));
                    lib_memory_copy(state.memory + 0x328u, new_descriptor, sizeof(new_descriptor));
                    state.cpu.data.gdtr.limit = 0x2fu;
                    state.cpu.data.tr.selector = 32u;
                    state.cpu.data.tr.base = 0x600u;
                    state.cpu.data.tr.limit = old32 ? 0x67u : 0x2bu;
                    state.cpu.data.tr.flagValid = LIB_TRUE;
                    state.cpu.data.tr.sys.type = old32 ?
                        VCPU_DESC_SYS_TYPE_TSS_32_BUSY : VCPU_DESC_SYS_TYPE_TSS_16_BUSY;
                    if (new32) {
                        lib_memory_copy(state.memory + 0xc1cu, &directory, 4u);
                        lib_memory_copy(state.memory + 0xc20u, &target, 4u);
                        lib_memory_copy(state.memory + 0xc24u, &flags, 4u);
                        lib_memory_copy(state.memory + 0xc38u, &stack, 4u);
                        lib_memory_copy(state.memory + 0xc4cu, &code_selector, 2u);
                        for (lib_u8 offset = 0x48u; offset <= 0x5cu; offset += 4u) {
                            if (offset != 0x4cu)
                                lib_memory_copy(state.memory + 0xc00u + offset, &data_selector, 2u);
                        }
                    } else {
                        state.memory[0x316u] = 0u; /* A 16-bit TSS supplies SP, not a usable ESP. */
                        lib_memory_copy(state.memory + 0xc0eu, &target, 2u);
                        lib_memory_copy(state.memory + 0xc10u, &flags, 2u);
                        lib_memory_copy(state.memory + 0xc1au, &stack, 2u);
                        lib_memory_copy(state.memory + 0xc24u, &code_selector, 2u);
                        lib_memory_copy(state.memory + 0xc22u, &data_selector, 2u);
                        lib_memory_copy(state.memory + 0xc26u, &data_selector, 2u);
                        lib_memory_copy(state.memory + 0xc28u, &data_selector, 2u);
                    }
                }
                if (outer_return) {
                    const lib_u32 directory = 0x8007u;
                    const lib_u32 pages[] = {0x0007u,0x9007u,0u,0xa007u};

                    lib_memory_copy(state.memory + 0x320u, state.memory + 0x308u, 8u);
                    lib_memory_copy(state.memory + 0x328u, state.memory + 0x310u, 8u);
                    state.memory[0x325u] = 0xfau;
                    state.memory[0x32du] = 0xf2u;
                    state.cpu.data.gdtr.limit = 0x2fu;
                    state.memory[0x472u] = (lib_u8)target_selector;
                    lib_memory_copy(state.memory + 0x7000u, &directory, sizeof(directory));
                    lib_memory_copy(state.memory + 0x8000u, pages, sizeof(pages));
                }
                if (mapped) {
                    const lib_u32 page = outer_return ? 0xb007u : 0xb003u;
                    lib_memory_copy(state.memory + 0x8008u, &page, sizeof(page));
                    state.memory[0xb000u] = 0xb8u; /* MOV AX/EAX, immediate: two components. */
                }
                if (form == TRANSFER_NEAR_JMP) {
                    code[0] = 0xe9u;
                    operand = target - (0x1ff0u + bytes);
                    lib_memory_copy(code + 1u, &operand, code32 ? 4u : 2u);
                } else if (form < TRANSFER_NEAR_JCC || form == TRANSFER_GATE_CALL) {
                    code[0] = form == TRANSFER_GATE_CALL ? 0x9au : 0xeau;
                    operand = form == TRANSFER_FAR_JMP ? target : 0u;
                    lib_memory_copy(code + 1u, &operand, code32 ? 4u : 2u);
                    code[bytes] = form == TRANSFER_FAR_JMP ? 8u : 24u;
                    bytes += 2u;
                } else if (form == TRANSFER_NEAR_JCC) {
                    code[0] = 0x0fu;
                    code[1] = 0x84u;
                    ++bytes;
                    operand = target - (0x1ff0u + bytes);
                    lib_memory_copy(code + 2u, &operand, code32 ? 4u : 2u);
                    state.cpu.data.eflags |= VCPU_EFLAGS_ZF;
                } else if (form == TRANSFER_SHORT_JCC) {
                    code[0] = 0x74u;
                    code[1] = 0x0eu;
                    bytes = 2u;
                    state.cpu.data.eflags |= VCPU_EFLAGS_ZF;
                } else if (form < TRANSFER_SAME_IRET) {
                    lib_u8 gate[] = {0,0x20u,8u,0,0,0,0,0};

                    gate[5] = form < TRANSFER_INT32 ? 0x86u : 0x8eu;
                    if (form == TRANSFER_TRAP16 || form == TRANSFER_TRAP32) ++gate[5];
                    lib_memory_copy(state.memory + 0x500u, gate, sizeof(gate));
                    state.cpu.data.idtr.limit = 0x107u;
                    code[0] = 0xcdu;
                    code[1] = 0x20u;
                    bytes = 2u;
                    if (form == TRANSFER_INT3 || form == TRANSFER_INTO) {
                        const lib_bool into = form == TRANSFER_INTO;
                        lib_memory_copy(state.memory + (into ? 0x420u : 0x418u),
                            gate, sizeof(gate));
                        code[0] = into ? 0xceu : 0xccu;
                        if (into) state.cpu.data.eflags |= VCPU_EFLAGS_OF;
                        bytes = 1u;
                    }
                } else if (form < TRANSFER_TASK16_TO16) {
                    const lib_bool iret = form == TRANSFER_SAME_IRET || form == TRANSFER_OUTER_IRET;
                    const lib_u32 frame[] = {
                        target, target_selector, iret ? 2u : 0x3100u,
                        iret ? 0x3100u : 43u, 43u
                    };
                    const lib_u8 slot_bytes = code32 ? 4u : 2u;
                    const lib_u8 slots = outer_return ? (iret ? 5u : 4u) :
                        (iret ? 3u : 2u);

                    code[0] = iret ? 0xcfu : 0xcbu;
                    bytes = 1u;
                    for (lib_u8 slot = 0u; slot < slots; ++slot) {
                        lib_memory_copy(state.memory + 0xa100u + slot * slot_bytes,
                            &frame[slot], slot_bytes);
                    }
                } else {
                    code[0] = 0xeau;
                    code[bytes] = 40u;
                    bytes += 2u;
                }
                lib_memory_copy(state.memory + 0x9ff0u, code, bytes);
                core_machine_cpu_execution_refresh(&state.execution);
                lib_memory_copy(&target_pte, state.memory + 0x8008u, sizeof(target_pte));
                if (state.execution.stop_requested || state.fault.valid ||
                    state.delivered_exception.valid || state.cpu.data.eip != target ||
                    state.cpu.data.cs.selector != target_selector || state.cpu.data.cr2 ||
                    target_pte != (mapped ? (outer_return ? 0xb007u : 0xb003u) : 0u)) {
                    lib_c_printf("TRANSFER-BOUNDARY:mode=%u:form=%u:eip=%x:cr2=%x:failed\n",
                        code32, form, state.cpu.data.eip, state.cpu.data.cr2);
                    return 1;
                }
                core_machine_cpu_timing_result timing;
                if (!core_machine_cpu_timing_select(&state.execution, &timing)) return 1;
                lib_memory_copy(&target_pte, state.memory + 0x8008u, sizeof(target_pte));
                if (timing.source_timing_unallocated || timing.ticks <= 1u ||
                    (!fixed_timing && !mapped && timing.retirement_origin !=
                        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL) ||
                    (!fixed_timing && mapped && (timing.retirement_origin ==
                        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL ||
                        timing.ticks != estimated_ticks + 1u)) ||
                    (fixed_timing && (timing.retirement_origin !=
                        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
                        timing.ticks != (form >= TRANSFER_TASK16_TO16 ?
                            task_clocks[form - TRANSFER_TASK16_TO16] : fixed_interrupt ? 59u :
                            form == TRANSFER_SAME_IRET ? 38u : form == TRANSFER_OUTER_IRET ? 82u : 69u))) ||
                    target_pte != (mapped ? (outer_return ? 0xb007u : 0xb003u) : 0u) ||
                    state.cpu.data.cr2 || state.fault.valid ||
                    state.delivered_exception.valid) {
                    lib_c_printf("TRANSFER-TIMING:mode=%u:form=%u:origin=%u:ticks=%llu:failed\n",
                        code32, form, timing.retirement_origin, timing.ticks);
                    return 1;
                }
                if (mapped) continue;
                estimated_ticks = timing.ticks;
                core_machine_cpu_execution_refresh(&state.execution);
                if (!state.delivered_exception.valid ||
                    state.delivered_exception.exception_mask != VCPUINS_EXCEPT_PF ||
                    state.delivered_exception.point.eip != target ||
                    state.cpu.data.cr2 != target || state.cpu.data.eip != 0x200u) {
                    lib_c_printf("TRANSFER-BOUNDARY:next-fetch:mode=%u:form=%u:eip=%x:cr2=%x:mask=%x:point=%x:failed\n",
                        code32, form, state.cpu.data.eip, state.cpu.data.cr2,
                        state.delivered_exception.exception_mask,
                        state.delivered_exception.point.eip);
                    lib_c_printf("TRANSFER-BOUNDARY:fault=%x:stop=%u:ss=%x:esp=%x:cr0=%x\n",
                        state.fault.exception_mask, state.execution.stop_requested,
                        state.cpu.data.ss.selector, state.cpu.data.esp, state.cpu.data.cr0);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static lib_i32 transfer_inner_gate(void)
{
    static cpu_instruction_fixture state;
    const lib_u32 directory = 0x8007u;
    const lib_u32 pages[] = {0x0007u,0x9007u,0u,0xa007u};
    const lib_u8 tss_descriptor[] = {0x67u,0,0,6u,0,0x8bu,0,0};
    const lib_u32 kernel_stack = 0x3200u;
    const lib_u16 kernel_ss = 16u;

    for (lib_u8 code32 = 0u; code32 < 2u; ++code32) {
        for (lib_u8 gate32 = 0u; gate32 < 2u; ++gate32) {
            for (lib_u8 interrupt = 0u; interrupt < 4u; ++interrupt) {
                lib_u8 code[] = {0x9au,0,0,0,0,0,0};
                lib_u8 gate[] = {0,0x20u,8u,0,0,0,0,0};
                lib_u32 target_pte;
                core_machine_cpu_timing_result timing;
                lib_u8 bytes = 7u;

                transfer_prepare(&state, code32 != 0u);
                lib_memory_copy(state.memory + 0x320u, state.memory + 0x308u, 8u);
                lib_memory_copy(state.memory + 0x328u, state.memory + 0x310u, 8u);
                state.memory[0x325u] = 0xfau;
                state.memory[0x32du] = 0xf2u;
                lib_memory_copy(state.memory + 0x330u, tss_descriptor, sizeof(tss_descriptor));
                lib_memory_copy(state.memory + 0x604u, &kernel_stack, sizeof(kernel_stack));
                lib_memory_copy(state.memory + 0x608u, &kernel_ss, sizeof(kernel_ss));
                state.cpu.data.gdtr.limit = 0x37u;
                state.cpu.data.tr.selector = 48u;
                state.cpu.data.tr.base = 0x600u;
                state.cpu.data.tr.limit = 0x67u;
                state.cpu.data.tr.flagValid = LIB_TRUE;
                state.cpu.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
                state.cpu.data.cs.selector = 35u;
                state.cpu.data.cs.dpl = 3u;
                state.cpu.data.ss.selector = 43u;
                state.cpu.data.ss.dpl = 3u;
                lib_memory_copy(state.memory + 0x7000u, &directory, sizeof(directory));
                lib_memory_copy(state.memory + 0x8000u, pages, sizeof(pages));
                if (interrupt) {
                    gate[5] = gate32 ? 0xeeu : 0xe6u;
                    lib_memory_copy(state.memory + 0x500u, gate, sizeof(gate));
                    state.cpu.data.idtr.limit = 0x107u;
                    code[0] = 0xcdu;
                    code[1] = 0x20u;
                    bytes = 2u;
                    if (interrupt >= 2u) {
                        const lib_bool into = interrupt == 3u;
                        lib_memory_copy(state.memory + (into ? 0x420u : 0x418u),
                            gate, sizeof(gate));
                        code[0] = into ? 0xceu : 0xccu;
                        if (into) state.cpu.data.eflags |= VCPU_EFLAGS_OF;
                        bytes = 1u;
                    }
                } else {
                    /* S11 owns operand/gate-width independence; this seam
                     * uses matching widths without concealing that receiver. */
                    if (code32 != gate32) continue;
                    gate[5] = gate32 ? 0xecu : 0xe4u;
                    lib_memory_copy(state.memory + 0x318u, gate, sizeof(gate));
                    bytes = code32 ? 7u : 5u;
                    code[bytes - 2u] = 24u;
                }
                lib_memory_copy(state.memory + 0x9ff0u, code, bytes);
                core_machine_cpu_execution_refresh(&state.execution);
                lib_memory_copy(&target_pte, state.memory + 0x8008u, sizeof(target_pte));
                if (state.execution.stop_requested || state.fault.valid ||
                    state.delivered_exception.valid || state.cpu.data.eip != 0x2000u ||
                    state.cpu.data.cs.selector != 8u || state.cpu.data.cs.dpl != 0u ||
                    state.cpu.data.ss.selector != 16u || state.cpu.data.cr2 || target_pte ||
                    !core_machine_cpu_timing_select(&state.execution, &timing) ||
                    timing.source_timing_unallocated || timing.ticks <= 1u ||
                    (interrupt && timing.ticks != 99u)) {
                    lib_c_printf("TRANSFER-INNER:code32=%u:gate32=%u:int=%u:eip=%x:cr2=%x:failed\n",
                        code32, gate32, interrupt, state.cpu.data.eip, state.cpu.data.cr2);
                    return 1;
                }
                core_machine_cpu_execution_refresh(&state.execution);
                if (!state.delivered_exception.valid ||
                    state.delivered_exception.exception_mask != VCPUINS_EXCEPT_PF ||
                    state.delivered_exception.point.eip != 0x2000u ||
                    state.cpu.data.cr2 != 0x2000u || state.cpu.data.eip != 0x200u) {
                    lib_c_printf("TRANSFER-INNER:next-fetch:code32=%u:gate32=%u:int=%u:failed\n",
                        code32, gate32, interrupt);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static lib_i32 transfer_reject_current_reference(void)
{
    static cpu_instruction_fixture state;
    const lib_u8 fault_gate[] = {0,2u,8u,0,0,0x8eu,0,0};

    for (lib_u8 code32 = 0u; code32 < 2u; ++code32) {
        for (lib_u8 form = 0u; form < 7u; ++form) {
            lib_u8 code[7] = {0};
            lib_u8 bytes = code32 ? 5u : 3u;
            const lib_u32 target = 0x2000u;
            const lib_u32 frame[] = {target,8u,2u};
            lib_u32 displacement = target - (0x1ff0u + bytes);
            lib_u32 target_pte;

            transfer_prepare(&state, code32 != 0u);
            lib_memory_copy(state.memory + 0x468u, fault_gate, sizeof(fault_gate));
            state.cpu.data.cs.limit = 0x1fffu;
            state.memory[0x309u] = 0x1fu;
            if (form == 0u) {
                code[0] = 0xe9u;
                lib_memory_copy(code + 1u, &displacement, code32 ? 4u : 2u);
            } else if (form < 4u) {
                code[0] = form == 3u ? 0x9au : 0xeau;
                if (form == 1u) lib_memory_copy(code + 1u, &target, code32 ? 4u : 2u);
                code[bytes] = form == 1u ? 8u : 24u;
                bytes += 2u;
                if (form == 3u && !code32) state.memory[0x31du] = 0x84u;
            } else if (form == 4u) {
                lib_u8 gate[] = {0,0x20u,8u,0,0,0x8eu,0,0};
                lib_memory_copy(state.memory + 0x500u, gate, sizeof(gate));
                state.cpu.data.idtr.limit = 0x107u;
                code[0] = 0xcdu;
                code[1] = 0x20u;
                bytes = 2u;
            } else {
                code[0] = form == 5u ? 0xcbu : 0xcfu;
                bytes = 1u;
                for (lib_u8 slot = 0u; slot < 3u; ++slot) {
                    lib_memory_copy(state.memory + 0xa100u + slot * (code32 ? 4u : 2u),
                        &frame[slot], code32 ? 4u : 2u);
                }
            }
            lib_memory_copy(state.memory + 0x9ff0u, code, bytes);
            core_machine_cpu_execution_refresh(&state.execution);
            lib_memory_copy(&target_pte, state.memory + 0x8008u, sizeof(target_pte));
            if (!state.delivered_exception.valid ||
                state.delivered_exception.exception_mask != VCPUINS_EXCEPT_GP ||
                state.delivered_exception.point.eip != 0x1ff0u ||
                state.cpu.data.cr2 || target_pte || state.cpu.data.eip != 0x200u) {
                lib_c_printf("TRANSFER-REJECT:code32=%u:form=%u:eip=%x:mask=%x:failed\n",
                    code32, form, state.cpu.data.eip, state.delivered_exception.exception_mask);
                return 1;
            }
        }
        transfer_prepare(&state, code32 != 0u);
        lib_memory_copy(state.memory + 0x460u, fault_gate, sizeof(fault_gate));
        state.cpu.data.ss.limit = 0x3100u;
        state.memory[0x9ff0u] = 0xcfu;
        core_machine_cpu_execution_refresh(&state.execution);
        if (!state.delivered_exception.valid ||
            state.delivered_exception.exception_mask != VCPUINS_EXCEPT_SS ||
            state.delivered_exception.point.eip != 0x1ff0u || state.cpu.data.cr2 ||
            state.cpu.data.eip != 0x200u) return 1;
    }
    return 0;
}

static lib_i32 transfer_unused_endpoints(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static cpu_instruction_fixture state;

    for (lib_u8 profile = 0u; profile < 5u; ++profile) {
        cpu_instruction_prepare(&state, profiles[profile]);
        state.cpu.data.cs.limit = 0u;
        state.cpu.data.ss.limit = 0u;
        state.cpu.data.esp = 0xfffeu;
        state.memory[0] = 0x90u;
        core_machine_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested || state.fault.valid ||
            state.delivered_exception.valid || state.cpu.data.eip != 1u ||
            state.cpu.data.esp != 0xfffeu) return 1;
    }
    return 0;
}

static lib_i32 transfer_restored_stack_adjustment(void)
{
    static cpu_instruction_fixture state;
    const lib_u8 instruction[] = {0xcau,8u,0};
    const lib_u32 directory = 0x8007u;
    const lib_u32 pages[] = {0x0007u,0x9007u,0xb007u,0xa007u};
    const lib_u32 frame[] = {0x2000u,35u};
    const lib_u32 stack[] = {0x3100u,43u};

    for (lib_u8 code32 = 0u; code32 < 2u; ++code32) {
        const lib_u8 slot_bytes = code32 ? 4u : 2u;

        transfer_prepare(&state, code32 != 0u);
        lib_memory_copy(state.memory + 0x320u, state.memory + 0x308u, 8u);
        lib_memory_copy(state.memory + 0x328u, state.memory + 0x310u, 8u);
        state.memory[0x325u] = 0xfau;
        state.memory[0x328u] = 0u;
        state.memory[0x329u] = 0x31u;
        state.memory[0x32du] = 0xf2u;
        state.cpu.data.gdtr.limit = 0x2fu;
        lib_memory_copy(state.memory + 0x7000u, &directory, sizeof(directory));
        lib_memory_copy(state.memory + 0x8000u, pages, sizeof(pages));
        lib_memory_copy(state.memory + 0x9ff0u, instruction, sizeof(instruction));
        state.memory[0xb000u] = 0x90u;
        for (lib_u8 slot = 0u; slot < 2u; ++slot) {
            lib_memory_copy(state.memory + 0xa100u + slot * slot_bytes,
                &frame[slot], slot_bytes);
            lib_memory_copy(state.memory + 0xa100u + (slot + 2u) * slot_bytes + 8u,
                &stack[slot], slot_bytes);
        }
        core_machine_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested || state.fault.valid ||
            state.delivered_exception.valid || state.cpu.data.eip != 0x2000u ||
            state.cpu.data.cs.selector != 35u || state.cpu.data.ss.selector != 43u ||
            state.cpu.data.esp != 0x3108u || state.cpu.data.ss.limit != 0x3100u)
            return 1;
        core_machine_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested || state.fault.valid ||
            state.delivered_exception.valid || state.cpu.data.eip != 0x2001u ||
            state.cpu.data.esp != 0x3108u) return 1;
    }
    return 0;
}

static lib_i32 transfer_next_term_limit(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static cpu_instruction_fixture state;
    const lib_u8 jump[] = {0xe9u, 0x1du, 0};
    const lib_u8 next[] = {0xb8u, 0, 0};

    for (lib_u8 profile = 0u; profile < 2u; ++profile) {
        lib_u64 estimated_ticks = 0u;

        for (lib_u8 complete = 0u; complete < 2u; ++complete) {
            core_machine_cpu_timing_result timing;

            cpu_instruction_prepare(&state, profiles[profile]);
            state.cpu.data.cs.limit = complete ? 0x22u : 0x20u;
            lib_memory_copy(state.memory, jump, sizeof(jump));
            lib_memory_copy(state.memory + 0x20u, next, sizeof(next));
            core_machine_cpu_execution_refresh(&state.execution);
            if (!core_machine_cpu_timing_select(&state.execution, &timing) ||
                state.cpu.data.eip != 0x20u || state.cpu.data.cr2 ||
                state.fault.valid || state.delivered_exception.valid ||
                state.execution.stop_requested || timing.source_timing_unallocated ||
                (!complete && timing.retirement_origin !=
                    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL) ||
                (complete && (timing.retirement_origin !=
                    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
                    timing.ticks != estimated_ticks + (profile ? 1u : 2u)))) {
                lib_c_printf("TRANSFER-TIMING:limit:profile=%u:complete=%u:failed\n",
                    profile, complete);
                return 1;
            }
            estimated_ticks = timing.ticks;
        }
    }
    return 0;
}

lib_i32 main(void)
{
    return transfer_unmapped_target() || transfer_inner_gate() ||
        transfer_reject_current_reference() || transfer_unused_endpoints() ||
        transfer_restored_stack_adjustment() ||
        transfer_next_term_limit();
}
