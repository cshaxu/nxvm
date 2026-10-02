#include "support/pic_fixture.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "support/core_machine_board_fixture.h"
#include "app-nxvm/devices/device_support.h"
#include "app-nxvm/devices/pic_bus.h"
#include <stdio.h>

typedef struct legacy_sreg_stack_machine { core_machine *machine; } legacy_sreg_stack_machine;

static lib_i32 legacy_sreg_stack_prepare(core_machine_cpu_profile profile, legacy_sreg_stack_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile, .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };
    lib_memory_set(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine) == LIB_STATUS_OK &&
        core_machine_freeze_execution_providers(state->machine) == LIB_STATUS_OK &&
        core_machine_reset(state->machine) == LIB_STATUS_OK &&
        core_machine_debug_patch_registers(state->machine, &entry) == LIB_STATUS_OK;
}

static lib_i32 legacy_sreg_stack_boot_protected(legacy_sreg_stack_machine *state)
{
    static const lib_u8 pointer[] = {0x37u,0u,0u,0x03u,0u,0u};
    static const lib_u8 gdt[] = {0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x98u,0,0,
        0xffu,0xffu,0,0x50u,0,0x12u,0,0};
    static const lib_u8 boot[] = {0x0fu,1u,0x16u,0,1u,0xb8u,1u,0,
        0x0fu,1u,0xf0u,0xb8u,0x10u,0,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0,0x8eu,0xd0u,0xbcu,0,0x80u,0xeau,0,0,8u,0};
    core_machine_run_result result;
    return core_machine_memory_write(state->machine,0x100u,pointer,sizeof(pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine,0x300u,gdt,sizeof(gdt)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine,0u,boot,sizeof(boot)) == LIB_STATUS_OK &&
        core_machine_run(state->machine,(core_machine_run_budget){10u,0u},&result) == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 10u;
}

static lib_i32 legacy_sreg_stack_limit(legacy_sreg_stack_machine *state,
    lib_u16 limit, lib_bool expdown)
{
    const lib_u8 encoded_limit[] = {(lib_u8)limit, (lib_u8)(limit >> 8)};
    const lib_u8 access = expdown ? 0x96u : 0x92u;
    const core_machine_debug_register_patch segment = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS),
        .values = { [CORE_MACHINE_DEBUG_SS] = 0x18u }
    };
    return core_machine_memory_write(state->machine, 0x318u, encoded_limit,
        sizeof(encoded_limit)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x31du, &access, 1u) == LIB_STATUS_OK &&
        core_machine_debug_patch_registers(state->machine, &segment) == LIB_STATUS_OK;
}

static lib_i32 legacy_sreg_stack_gprs_same_except_esp(const core_machine_debug_cpu_snapshot *before,
    const core_machine_debug_cpu_snapshot *after)
{
    return before->eax == after->eax &&
        before->ecx == after->ecx &&
        before->edx == after->edx &&
        before->ebx == after->ebx &&
        before->ebp == after->ebp &&
        before->esi == after->esi &&
        before->edi == after->edi;
}

static lib_i32 legacy_sreg_stack_sregs_same(const core_machine_debug_cpu_snapshot *before, const core_machine_debug_cpu_snapshot *after)
{
    return lib_memory_compare(&before->es, &after->es,
        sizeof(before->es)) == 0 &&
        lib_memory_compare(&before->cs, &after->cs,
        sizeof(before->cs)) == 0 &&
        lib_memory_compare(&before->ss, &after->ss,
        sizeof(before->ss)) == 0 &&
        lib_memory_compare(&before->ds, &after->ds,
        sizeof(before->ds)) == 0 &&
        lib_memory_compare(&before->fs, &after->fs,
        sizeof(before->fs)) == 0 &&
        lib_memory_compare(&before->gs, &after->gs,
        sizeof(before->gs)) == 0;
}

static const core_machine_debug_segment_snapshot *legacy_sreg_stack_target(const core_machine_debug_cpu_snapshot *cpu,
    lib_u8 target)
{
    if (target == 0u)
        return &cpu->es;
    if (target == 1u)
        return &cpu->ss;
    return &cpu->ds;
}

static lib_i32 legacy_sreg_stack_non_target_sregs_same(const core_machine_debug_cpu_snapshot *before,
    const core_machine_debug_cpu_snapshot *after, lib_u8 target)
{
    return (target == 0u || lib_memory_compare(&before->es, &after->es,
        sizeof(before->es)) == 0) &&
        lib_memory_compare(&before->cs, &after->cs,
        sizeof(before->cs)) == 0 &&
        (target == 1u || lib_memory_compare(&before->ss, &after->ss,
        sizeof(before->ss)) == 0) &&
        (target == 2u || lib_memory_compare(&before->ds, &after->ds,
        sizeof(before->ds)) == 0) &&
        lib_memory_compare(&before->fs, &after->fs,
        sizeof(before->fs)) == 0 &&
        lib_memory_compare(&before->gs, &after->gs,
        sizeof(before->gs)) == 0;
}

static lib_i32 legacy_sreg_stack_real_cache(const core_machine_debug_segment_snapshot *sreg, lib_u16 selector)
{
    return sreg->selector == selector &&
        sreg->base == (lib_u32)selector << 4u && sreg->limit == 0xffffu &&
        !sreg->executable && sreg->writable &&
        !sreg->big && !sreg->expdown;
}

static lib_i32 legacy_sreg_stack_test_protected_ss_null(void)
{
    legacy_sreg_stack_machine state; core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic; core_machine_debug_cpu_snapshot before,after; lib_status status;
    lib_u16 selector=0u; lib_i32 failed=!legacy_sreg_stack_prepare(CORE_MACHINE_CPU_PROFILE_80386,&state);
    if(!failed) failed|=!legacy_sreg_stack_boot_protected(&state);
    if(!failed) {
        failed|=core_machine_memory_write(state.machine,0xc000u,&selector,2u)!=LIB_STATUS_OK ||
            core_machine_memory_write(state.machine,0x2000u,(lib_u8[]){0x17u},1u)!=LIB_STATUS_OK;
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;

        status=core_machine_run(state.machine,(core_machine_run_budget){1u,0u},&result);
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        failed|=core_machine_get_cpu_diagnostic(state.machine,&diagnostic)!=LIB_STATUS_OK ||
            status!=LIB_STATUS_INTERNAL_ERROR || !diagnostic.first_fault.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,(1u << 8)) || after.eip!=0u ||
            after.esp!=before.esp || after.eflags!=before.eflags ||
            !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
            !legacy_sreg_stack_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine); return !failed;
}

static lib_i32 legacy_sreg_stack_test_protected_rejects(void)
{
    static const lib_u8 opcodes[] = {0x07u,0x17u,0x1fu};
    static const lib_u16 selectors[] = {0x28u,0x23u,0x30u};
    lib_u8 target,kind;
    for(target=0u;target!=3u;++target) for(kind=0u;kind!=3u;++kind) {
        legacy_sreg_stack_machine state; core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic; core_machine_debug_cpu_snapshot before,after; lib_status status;
        lib_u16 selector=selectors[kind], observed=0u;
        lib_i32 failed=!legacy_sreg_stack_prepare(CORE_MACHINE_CPU_PROFILE_80386,&state);
        if(!failed) failed|=!legacy_sreg_stack_boot_protected(&state);
        if(!failed) {
            failed|=core_machine_memory_write(state.machine,0xc000u,&selector,2u)!=LIB_STATUS_OK ||
                core_machine_memory_write(state.machine,0x2000u,&opcodes[target],1u)!=LIB_STATUS_OK;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;

            status=core_machine_run(state.machine,(core_machine_run_budget){1u,0u},&result);
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            failed|=core_machine_get_cpu_diagnostic(state.machine,&diagnostic)!=LIB_STATUS_OK ||
                status!=LIB_STATUS_INTERNAL_ERROR || !diagnostic.first_fault.valid ||
                !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,(1u << 8)) || after.eip!=0u ||
                after.esp!=before.esp || after.eax!=before.eax || after.ecx!=before.ecx ||
                after.edx!=before.edx || after.ebx!=before.ebx || after.ebp!=before.ebp ||
                after.esi!=before.esi || after.edi!=before.edi || after.eflags!=before.eflags ||
                !legacy_sreg_stack_sregs_same(&before, &after) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,0xc000u,CORE_MACHINE_REFERENCE_OF(observed),2u)!=LIB_STATUS_OK || observed!=selector;
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }return 1;
}

static lib_i32 legacy_sreg_stack_test_protected_stack_limits(void)
{
    static const lib_u8 opcodes[] = {0x06u,0x07u};
    lib_u8 form;
    for(form=0u;form!=2u;++form) {
        legacy_sreg_stack_machine state; core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic; core_machine_debug_cpu_snapshot before,after; lib_status status;
        lib_u16 image=0xbe5au; lib_u32 candidate=form==0u?0xbffeu:0xc000u;
        lib_i32 failed=!legacy_sreg_stack_prepare(CORE_MACHINE_CPU_PROFILE_80386,&state);
        if(!failed) failed|=!legacy_sreg_stack_boot_protected(&state);
        if(!failed) {
            failed |= !legacy_sreg_stack_limit(&state,
                form == 0u ? 0xffffu : 0x7fffu, form == 0u);
            failed|=core_machine_memory_write(state.machine,candidate,&image,2u)!=LIB_STATUS_OK ||
                core_machine_memory_write(state.machine,0x2000u,&opcodes[form],1u)!=LIB_STATUS_OK;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;

            status=core_machine_run(state.machine,(core_machine_run_budget){1u,0u},&result);
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            failed|=core_machine_get_cpu_diagnostic(state.machine,&diagnostic)!=LIB_STATUS_OK ||
                status!=LIB_STATUS_INTERNAL_ERROR || !diagnostic.first_fault.valid ||
                !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,(1u << 8)) ||
                after.eip!=0u || after.esp!=before.esp ||
                after.eflags!=before.eflags || lib_memory_compare(&before.es,&after.es,sizeof(before.es))!=0 ||
                !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                !legacy_sreg_stack_sregs_same(&before, &after) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,candidate,CORE_MACHINE_REFERENCE_OF(image),2u)!=LIB_STATUS_OK || image!=0xbe5au;
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }
    return 1;
}

static lib_i32 legacy_sreg_stack_test_irq(void)
{
    static const lib_u8 codes[][3]={{0x17u,0x90u},{0x07u,0x90u},{0x1fu,0x90u},{0x06u,0x90u}};
    static const lib_u8 frame[] = {2u,1u,1u,1u}; static const lib_u8 halt=0xf4u;
    lib_u8 form;
    for(form=0u;form!=4u;++form) {
        legacy_sreg_stack_machine state; core_machine_pic_irq_source source;
        core_machine_run_result result; core_machine_debug_cpu_snapshot before,after;
        lib_u16 off=0x100u,seg=0u,ip=0u,sel=0u,image=0u;
        lib_i32 failed=!legacy_sreg_stack_prepare(CORE_MACHINE_CPU_PROFILE_80386,&state);
        if(!failed) {
            const core_machine_debug_register_patch registers = {
                .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
                .values = { [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
                    [CORE_MACHINE_DEBUG_EFLAGS] = 0x202u }
            };
            failed |= core_machine_debug_patch_registers(state.machine, &registers) != LIB_STATUS_OK;
            if(form!=3u) failed|=core_machine_memory_write(state.machine,0x8000u,&sel,2u)!=LIB_STATUS_OK;
            failed|=core_machine_memory_write(state.machine,0u,codes[form],2u)!=LIB_STATUS_OK ||
                core_machine_memory_write(state.machine,0x80u,&off,2u)!=LIB_STATUS_OK ||
                core_machine_memory_write(state.machine,0x82u,&seg,2u)!=LIB_STATUS_OK ||
                core_machine_memory_write(state.machine,0x100u,&halt,1u)!=LIB_STATUS_OK;
        }
        if(!failed) {
            lib_memory_set(&source,0,sizeof(source));
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
            test_pic_program_vector(&state.machine->board->shared_pic_master, 0x20u);
            core_machine_pic_irq_source_bind(&source,&state.machine->board->shared_pic_master,&state.machine->board->shared_pic_slave,0u);
            core_machine_pic_irq_source_assert(&source);core_machine_pic_irq_source_deassert(&source);
            failed|=core_machine_run(state.machine,(core_machine_run_budget){3u,0u},&result)!=LIB_STATUS_OK ||
                result.reason!=CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            failed|=core_machine_memory_read_physical(&state.machine->executor_memory,after.ss.base+(lib_u16)after.esp,CORE_MACHINE_REFERENCE_OF(ip),2u)!=LIB_STATUS_OK ||
                after.eip!=0x101u || ip!=frame[form] || !CORE_MACHINE_BIT_IS_SET(test_pic_read(&state.machine->board->shared_pic_master, 0x0bu),VPIC_ISR_IRQ(0u)) ||
                CORE_MACHINE_BIT_IS_SET(test_pic_read(&state.machine->board->shared_pic_master, 0x0au),VPIC_IRR_IRQ(0u)) ||
                !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                (form==3u && (after.esp!=0x7ff8u ||
                !legacy_sreg_stack_sregs_same(&before, &after) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                0x7ffeu,CORE_MACHINE_REFERENCE_OF(image),2u)!=LIB_STATUS_OK ||
                image!=before.es.selector)) ||
                (form!=3u && (after.esp!=0x7ffcu ||
                !legacy_sreg_stack_non_target_sregs_same(&before,&after,
                form==0u?1u:form==1u?0u:2u) ||
                !legacy_sreg_stack_real_cache(legacy_sreg_stack_target(&after,
                form==0u?1u:form==1u?0u:2u), 0u)));
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }return 1;
}

lib_i32 main(void)
{
    if (!legacy_sreg_stack_test_protected_ss_null()) {
        printf("LEGACY-SREG-STACK-BOARD stage=ss-null\n"); return 1;
    }
    if (!legacy_sreg_stack_test_protected_rejects()) {
        printf("LEGACY-SREG-STACK-BOARD stage=rejects\n"); return 1;
    }
    if (!legacy_sreg_stack_test_protected_stack_limits()) {
        printf("LEGACY-SREG-STACK-BOARD stage=stack-limits\n"); return 1;
    }
    if (!legacy_sreg_stack_test_irq()) {
        printf("LEGACY-SREG-STACK-BOARD stage=irq\n"); return 1;
    }
    printf("M5:T539:S26:LEGACY-SREG-STACK-BOARD:OK\n");
    return 0;
}
