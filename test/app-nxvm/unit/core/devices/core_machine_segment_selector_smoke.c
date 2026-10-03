#include "lib/types/types_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "support/core_machine_board_fixture.h"

#define SEG_GDT_POINTER 0x0100u
#define SEG_GDT_ADDRESS 0x0300u
#define SEG_CODE_ADDRESS 0x2000u
#define SEG_DATA_ADDRESS 0x3000u

typedef struct segment_machine {
    core_machine *machine;
} segment_machine;


static lib_i32 segment_prepare(segment_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine, LIB_NULL) != LIB_STATUS_OK) return 0;
    if (core_machine_freeze_execution_providers(state->machine) != LIB_STATUS_OK ||
        core_machine_reset(state->machine) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(state->machine,
            &(core_machine_debug_register_patch){
                .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
            }) != LIB_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 segment_write(segment_machine *state, lib_u32 address,
    const void *bytes, lib_size byte_count)
{
    return state != LIB_NULL && state->machine != LIB_NULL &&
        core_machine_memory_write(state->machine, address, bytes, byte_count) ==
            LIB_STATUS_OK;
}


static lib_i32 segment_capture(segment_machine *state,
    core_machine_debug_cpu_snapshot *snapshot)
{
    return core_machine_debug_capture_cpu_snapshot(state->machine,
        CORE_MACHINE_CPU_SNAPSHOT_CURRENT, snapshot) == LIB_STATUS_OK;
}

static lib_i32 segment_patch(segment_machine *state,
    core_machine_debug_register register_id, lib_u32 value)
{
    core_machine_debug_register_patch patch = {0};
    patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(register_id);
    patch.values[register_id] = value;
    return core_machine_debug_patch_registers(state->machine, &patch) == LIB_STATUS_OK;
}

static lib_i32 segment_run_exception(segment_machine *state, const lib_u8 *code,
    lib_size code_size, lib_u32 address, lib_u32 exception,
    core_machine_debug_cpu_snapshot *out_cpu)
{
    const core_machine_run_budget budget = { 16u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;

    if (state == LIB_NULL || state->machine == LIB_NULL || code == LIB_NULL ||
        out_cpu == LIB_NULL || !segment_write(state, address, code, code_size))
        return 0;
    if (!segment_patch(state, CORE_MACHINE_DEBUG_EIP,
            address == 0u ? 0u : address - SEG_CODE_ADDRESS)) return 0;
    if (exception == VCPUINS_EXCEPT_TS || exception == VCPUINS_EXCEPT_NP ||
        exception == VCPUINS_EXCEPT_SS || exception == VCPUINS_EXCEPT_GP) {
        exception = VCPUINS_EXCEPT_DF;
    }
    if (core_machine_run(state->machine, budget, &result) != LIB_STATUS_INTERNAL_ERROR ||
        result.reason != CORE_MACHINE_STOP_FAULT ||
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) !=
            LIB_STATUS_OK || !diagnostic.first_fault.valid ||
        !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask, exception)) return 0;
    return segment_capture(state, out_cpu);
}

static lib_i32 segment_boot_protected(segment_machine *state)
{
    static const lib_u8 gdt_pointer[] = {
        0x37u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0x40,0,
        0xff,0xff,0,0x30,0,0x92,0x40,0,
        0xff,0xff,0,0x30,0,0x12,0x40,0,
        0xff,0xff,0,0x30,0,0x98,0x40,0,
        0,0,0,0,0,0x80,0,0,
        0xff,0xff,0,0x00,0,0x89,0x40,0
    };
    static const lib_u8 real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    const core_machine_run_budget budget = { 9u, 0u };
    core_machine_run_result result;
    lib_i32 installed;
    lib_status run_status;

    installed = segment_write(state, SEG_GDT_POINTER, gdt_pointer,
        sizeof(gdt_pointer));
    installed &= segment_write(state, SEG_GDT_ADDRESS, gdt, sizeof(gdt));
    installed &= segment_write(state, 0u, real_code, sizeof(real_code));
    if (!installed) {
        fprintf(stderr,
            "M5:T301:SEGMENT-SELECTOR bootstrap-install-failed\n");
        return 0;
    }
    run_status = core_machine_run(state->machine, budget, &result);
    if (run_status != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 9u) {
        fprintf(stderr,
            "M5:T301:SEGMENT-SELECTOR bootstrap status=%d reason=%d detail=%08x\n",
            run_status, result.reason, result.detail);
        return 0;
    }
    return 1;
}

static lib_i32 segment_boot_protected_286(segment_machine *state)
{
    static const lib_u8 gdt_pointer[] = {
        0x37u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const lib_u8 idt_pointer[] = {
        0x6fu, 0x00u, 0x00u, 0x04u, 0x00u, 0x00u
    };
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0xff,0xff,0,0x30,0,0x12,0,0,
        0xff,0xff,0,0x30,0,0x98,0,0,
        0x0fu,0,0,0x50u,0,0x82u,0,0,
        0xff,0xff,0,0x00,0,0x89,0,0
    };
    static const lib_u8 real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    const core_machine_run_budget budget = { 10u, 0u };
    core_machine_run_result result;
    lib_u8 idt[0x70u] = { 0u };

    idt[11u * 8u + 1u] = 0x01u;
    idt[11u * 8u + 2u] = 0x08u;
    idt[11u * 8u + 5u] = 0x86u;
    idt[12u * 8u + 1u] = 0x01u;
    idt[12u * 8u + 2u] = 0x08u;
    idt[12u * 8u + 5u] = 0x86u;
    idt[13u * 8u + 1u] = 0x01u;
    idt[13u * 8u + 2u] = 0x08u;
    idt[13u * 8u + 5u] = 0x86u;

    if (!segment_write(state, SEG_GDT_POINTER, gdt_pointer,
            sizeof(gdt_pointer)) || !segment_write(state, SEG_GDT_ADDRESS, gdt,
            sizeof(gdt)) || !segment_write(state, 0x0110u, idt_pointer,
            sizeof(idt_pointer)) || !segment_write(state, 0x0400u, idt,
            sizeof(idt)) || !segment_write(state, 0u, real_code,
            sizeof(real_code)) || !segment_write(state, SEG_CODE_ADDRESS + 0x100u,
            &(const lib_u8){0xf4u}, 1u)) return 0;
    return core_machine_run(state->machine, budget, &result) == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 10u;
}

static const core_machine_debug_segment_snapshot *segment_sreg(const core_machine_debug_cpu_snapshot *cpu, lib_u8 target)
{
    if (cpu == LIB_NULL) return LIB_NULL;
    switch (target) {
    case 0u: return &cpu->es;
    case 1u: return &cpu->ds;
    case 2u: return &cpu->ss;
    case 3u: return &cpu->fs;
    case 4u: return &cpu->gs;
    default: return LIB_NULL;
    }
}

static lib_i32 segment_test_80286_protected_cache_rejections(void)
{
    static const lib_u8 nonpresent_ds[] = {
        0xb8u,0x18u,0x00u,0x8eu,0xd8u
    };
    static const lib_u8 execute_only_ds[] = {
        0xb8u,0x20u,0x00u,0x8eu,0xd8u
    };
    static const lib_u8 null_ds[] = {
        0xb8u,0x00u,0x00u,0x8eu,0xd8u,0xf4u
    };
    static const lib_u8 null_ss[] = {
        0xb8u,0x00u,0x00u,0x8eu,0xd0u
    };
    const lib_u8 *codes[] = {
        nonpresent_ds, execute_only_ds, null_ds, null_ss
    };
    const lib_size sizes[] = {
        sizeof(nonpresent_ds), sizeof(execute_only_ds), sizeof(null_ds),
        sizeof(null_ss)
    };
    const lib_u32 exceptions[] = {
        VCPUINS_EXCEPT_NP, VCPUINS_EXCEPT_GP, 0u, VCPUINS_EXCEPT_GP
    };
    lib_u8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form) {
        segment_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_debug_cpu_snapshot before = {0};
        core_machine_debug_cpu_snapshot after = {0};
        lib_i32 failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286) ||
            !segment_boot_protected_286(&state);

        if (!failed) {
            failed |= !segment_capture(&state, &before);
            failed |= !segment_write(&state, SEG_CODE_ADDRESS, codes[form],
                sizes[form]);
            failed |= !segment_patch(&state, CORE_MACHINE_DEBUG_EIP, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ form == 2u ? 16u : 2u, 0u },
                &result) != LIB_STATUS_OK ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
            failed |= !segment_capture(&state, &after);
            if (form == 2u) {
                failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                    diagnostic.first_fault.valid || after.ds.selector != 0u ||
                    after.eax != (lib_u32)(codes[form][1] | (codes[form][2] << 8u)) ||
                    lib_memory_compare(&after.es, &before.es,
                    sizeof(after.es)) != 0 ||
                    lib_memory_compare(&after.ss, &before.ss,
                    sizeof(after.ss)) != 0;
            } else {
                const core_machine_debug_segment_snapshot *target = form == 3u ? &after.ss :
                    &after.ds;
                const core_machine_debug_segment_snapshot *before_target = form == 3u ?
                    &before.ss : &before.ds;

                failed |= result.reason != CORE_MACHINE_STOP_BUDGET ||
                    diagnostic.first_fault.valid ||
                    !diagnostic.last_delivered_exception.valid ||
                    !CORE_MACHINE_BIT_IS_SET(diagnostic.last_delivered_exception.exception_mask,
                    exceptions[form]) || after.eip != 0x100u ||
                    lib_memory_compare(target, before_target, sizeof(*target)) != 0 ||
                    after.eax != (lib_u32)(codes[form][1] | (codes[form][2] << 8u)) ||
                    lib_memory_compare(&after.es, &before.es,
                    sizeof(after.es)) != 0;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 1;
    }
    return 0;
}

typedef struct segment_lxs_form {
    lib_u8 first;
    lib_u8 second;
    lib_u8 bytes;
    lib_u8 target;
} segment_lxs_form;

static lib_i32 segment_test_lxs_fault_atomicity(void)
{
    static const segment_lxs_form forms[] = {
        { 0xc4u,0u,1u,0u }, { 0xc5u,0u,1u,1u },
        { 0x0fu,0xb2u,2u,2u }, { 0x0fu,0xb4u,2u,3u },
        { 0x0fu,0xb5u,2u,4u }
    };
    static const lib_u8 pointer[] = { 0x44u,0x33u,0x22u,0x11u,0x18u,0u };
    lib_size index;
    lib_i32 failed = 0;

    for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
        lib_u8 code[8u] = {0};
        lib_u8 code_size = 0u;
        segment_machine state;
        core_machine_debug_cpu_snapshot before = {0};
        core_machine_debug_cpu_snapshot after = {0};
        lib_u32 exception = forms[index].target == 2u ? VCPUINS_EXCEPT_SS :
            VCPUINS_EXCEPT_NP;

        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !segment_boot_protected(&state)) return 1;
        code[code_size++] = forms[index].first;
        if (forms[index].bytes == 2u) code[code_size++] = forms[index].second;
        code[code_size++] = 0x05u;
        code[code_size++] = 0x00u;
        code[code_size++] = 0x04u;
        code[code_size++] = 0x00u;
        code[code_size++] = 0x00u;
        failed |= !segment_capture(&state, &before);
        failed |= !segment_write(&state, SEG_DATA_ADDRESS + 0x0400u, pointer,
                sizeof(pointer)) || !segment_run_exception(&state, code,
                code_size, SEG_CODE_ADDRESS, exception, &after) ||
            before.eax != after.eax ||
            before.esp != after.esp ||
            before.eflags != after.eflags ||
            lib_memory_compare(&before.es, &after.es,
                sizeof(before.es)) != 0 ||
            lib_memory_compare(&before.ds, &after.ds,
                sizeof(before.ds)) != 0 ||
            lib_memory_compare(&before.ss, &after.ss,
                sizeof(before.ss)) != 0 ||
            lib_memory_compare(&before.fs, &after.fs,
                sizeof(before.fs)) != 0 ||
            lib_memory_compare(&before.gs, &after.gs,
                sizeof(before.gs)) != 0;
        core_machine_destroy(state.machine);
    }
    return failed;
}

typedef struct segment_sreg_failure {
    lib_u8 target;
    lib_u8 mov_modrm;
    lib_u16 selector;
    lib_u32 exception;
    lib_u32 access_address;
    lib_u8 access_value;
} segment_sreg_failure;

static lib_i32 segment_test_protected_sreg_failures(void)
{
    static const segment_sreg_failure failures[] = {
        { 1u,0xd8u,0x0018u,VCPUINS_EXCEPT_NP,SEG_GDT_ADDRESS + 29u,0x12u },
        { 2u,0xd0u,0x0018u,VCPUINS_EXCEPT_SS,SEG_GDT_ADDRESS + 29u,0x12u },
        { 3u,0xe0u,0x0020u,VCPUINS_EXCEPT_GP,SEG_GDT_ADDRESS + 37u,0x98u },
        { 4u,0xe8u,0x0013u,VCPUINS_EXCEPT_GP,SEG_GDT_ADDRESS + 21u,0x93u }
    };
    static const lib_u8 pop_fs[] = { 0x66u,0x0fu,0xa1u };
    static const lib_u8 pop_ss[] = { 0x66u,0x17u };
    static const lib_u8 selector_nonpresent[] = { 0x18u,0,0,0 };
    lib_size index;
    lib_i32 failed = 0;

    for (index = 0u; index < sizeof(failures) / sizeof(failures[0]); ++index) {
        lib_u8 code[] = { 0xb8u,0,0,0,0,0x8eu,failures[index].mov_modrm };
        segment_machine state;
        core_machine_debug_cpu_snapshot before = {0};
        core_machine_debug_cpu_snapshot after = {0};
        const core_machine_debug_segment_snapshot *before_sreg;
        const core_machine_debug_segment_snapshot *after_sreg;
        lib_i32 case_failed;

        code[1u] = (lib_u8)failures[index].selector;
        code[2u] = (lib_u8)(failures[index].selector >> 8u);
        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !segment_boot_protected(&state)) return 1;
        case_failed = !segment_capture(&state, &before) ||
            !segment_run_exception(&state, code, sizeof(code),
            SEG_CODE_ADDRESS, failures[index].exception, &after);
        before_sreg = segment_sreg(&before, failures[index].target);
        after_sreg = segment_sreg(&after, failures[index].target);
        case_failed |= before_sreg == LIB_NULL || after_sreg == LIB_NULL ||
            lib_memory_compare(before_sreg, after_sreg, sizeof(*before_sreg)) != 0 ||
            before.esp != after.esp ||
            before.eflags != after.eflags;
        if (case_failed) fprintf(stderr,
            "M5:T539:S28:SEGMENT-SELECTOR mov-fail index=%u selector=%04x esp=%08x/%08x flags=%08x/%08x\n",
            (unsigned)index, failures[index].selector, before.esp,
            after.esp, before.eflags, after.eflags);
        failed |= case_failed;
        core_machine_destroy(state.machine);
    }
    for (index = 0u; index < 2u; ++index) {
        const lib_u8 *code = index == 0u ? pop_fs : pop_ss;
        lib_size code_size = index == 0u ? sizeof(pop_fs) : sizeof(pop_ss);
        lib_u8 target = index == 0u ? 3u : 2u;
        lib_u32 exception = index == 0u ? VCPUINS_EXCEPT_NP : VCPUINS_EXCEPT_SS;
        segment_machine state;
        core_machine_debug_cpu_snapshot before = {0};
        core_machine_debug_cpu_snapshot after = {0};
        const core_machine_debug_segment_snapshot *before_sreg;
        const core_machine_debug_segment_snapshot *after_sreg;
        lib_i32 case_failed;

        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !segment_boot_protected(&state)) return 1;
        if (!segment_patch(&state, CORE_MACHINE_DEBUG_ESP, 0x8000u)) return 1;
        case_failed = !segment_capture(&state, &before);
        case_failed |= !segment_write(&state, SEG_DATA_ADDRESS + 0x8000u,
                selector_nonpresent, sizeof(selector_nonpresent)) ||
            !segment_run_exception(&state, code, code_size, SEG_CODE_ADDRESS,
                exception, &after);
        before_sreg = segment_sreg(&before, target);
        after_sreg = segment_sreg(&after, target);
        case_failed |= before_sreg == LIB_NULL || after_sreg == LIB_NULL ||
            lib_memory_compare(before_sreg, after_sreg, sizeof(*before_sreg)) != 0 ||
            before.esp != after.esp ||
            before.eflags != after.eflags;
        if (case_failed) fprintf(stderr,
            "M5:T539:S28:SEGMENT-SELECTOR pop-fail index=%u esp=%08x/%08x flags=%08x/%08x\n",
            (unsigned)index, before.esp, after.esp,
            before.eflags, after.eflags);
        failed |= case_failed;
        core_machine_destroy(state.machine);
    }
    return failed;
}

static lib_i32 segment_test_pop_fault_atomicity(void)
{
    static const lib_u8 pop_fs[] = { 0x66u,0x0fu,0xa1u };
    static const lib_u8 selector[] = { 0x18u,0x00u,0,0 };
    const core_machine_run_budget budget = { 8u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    segment_machine state;
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_i32 failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) failed |= !segment_boot_protected(&state);
    if (!failed) {
        failed |= !segment_patch(&state, CORE_MACHINE_DEBUG_ESP, 0x8000u);
        failed |= !segment_capture(&state, &before);
        failed |= !segment_write(&state, SEG_DATA_ADDRESS + 0x8000u, selector,
            sizeof(selector)) || !segment_write(&state, SEG_CODE_ADDRESS, pop_fs,
            sizeof(pop_fs));
        failed |= !segment_patch(&state, CORE_MACHINE_DEBUG_EIP, 0u);
        failed |= core_machine_run(state.machine, budget, &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK || !diagnostic.first_fault.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            diagnostic.first_fault.exception_code != 0u;
        failed |= !segment_capture(&state, &after);
        failed |= after.esp != before.esp ||
            lib_memory_compare(&after.fs, &before.fs, sizeof(after.fs)) != 0;
    }
    core_machine_destroy(state.machine);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 rejected_286 = segment_test_80286_protected_cache_rejections();
    lib_i32 lxs_faults = segment_test_lxs_fault_atomicity();
    lib_i32 sreg_faults = segment_test_protected_sreg_failures();
    lib_i32 pop_fault = segment_test_pop_fault_atomicity();

    if (rejected_286 || lxs_faults || sreg_faults || pop_fault) {
        fprintf(stderr, "M5:T539:S28:SEGMENT-SELECTOR-BOARD:FAIL 286=%d lxs=%d sreg=%d pop=%d\n",
            rejected_286, lxs_faults, sreg_faults, pop_fault);
        return 1;
    }
    printf("M5:T539:S28:SEGMENT-SELECTOR-BOARD:OK\n");
    return 0;
}
