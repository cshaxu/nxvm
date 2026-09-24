/* Copyright 2012-2014 Neko. */

/* VCPU defines the Central Processing Unit. */
#include "lib/types/types_interface.h"
#include "app-nxvm/devices/device_support.h"


#include "app-nxvm/devices/cpu_instructions.h"

#include "app-nxvm/devices/transaction.h"

#include "app-nxvm/devices/cpu.h"

#define cpu_state (*context->cpu)
#define instruction_state (*context->instructions)

static lib_u32 core_machine_cpu_reset_code_base(
    core_machine_cpu_profile profile)
{
    switch (profile) {
    case CORE_MACHINE_CPU_PROFILE_8086:
    case CORE_MACHINE_CPU_PROFILE_8088:
    case CORE_MACHINE_CPU_PROFILE_80186:
        return 0x000f0000u;
    case CORE_MACHINE_CPU_PROFILE_80286:
        return 0x00ff0000u;
    case CORE_MACHINE_CPU_PROFILE_DEFAULT:
    case CORE_MACHINE_CPU_PROFILE_80386:
        return 0xffff0000u;
    }
    return 0xffff0000u;
}

void core_machine_cpu_execution_context_initialize(
    core_machine_cpu_execution_context *context, t_cpu *cpu,
    t_cpuins *instructions, t_ram *memory, t_port *port)
{
    if (context == LIB_NULL) return;
    context->cpu = cpu;
    context->instructions = instructions;
    context->memory = memory;
    context->port = port;
    context->transaction = LIB_NULL;
    context->pic_master = LIB_NULL;
    context->pic_slave = LIB_NULL;
    context->diagnostic_provider = LIB_NULL;
    context->diagnostic_context = LIB_NULL;
    context->external_cycle_provider = LIB_NULL;
    context->external_cycle_context = LIB_NULL;
    context->firmware_interrupt_provider = LIB_NULL;
    context->firmware_interrupt_context = LIB_NULL;
    context->stop_requested = LIB_FALSE;
    context->debug_pause_requested = LIB_FALSE;
    context->reset_requested = LIB_FALSE;
    context->debug_trap_pending = LIB_FALSE;
    context->debug_tf_before = LIB_FALSE;
    context->debug_rf_before = LIB_FALSE;
    context->debug_trap_cause = 0u;
    context->preview_mode = LIB_FALSE;
    context->memory_access_provenance = CORE_MACHINE_CPU_MEMORY_ACCESS_DATA;
    context->prefetch_count = 0u;
    context->prefetch_capacity = 15u;
    context->prefetch_valid = LIB_FALSE;
    context->prefetch_expected_valid = LIB_FALSE;
    context->prefetch_reservation_valid = LIB_FALSE;
    context->prefetch_reservation_linear = 0u;
    context->prefetch_reservation_count = 0u;
    context->cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    context->fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE;
    context->cpu_80386_cr_mov_ignores_mod = LIB_FALSE;
    context->fpu = LIB_NULL;
}

void core_machine_cpu_execution_context_bind_profiles(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_profile cpu_profile,
    core_machine_fpu_profile fpu_profile,
    lib_u8 cpu_80386_cr_mov_ignores_mod)
{
    if (context == LIB_NULL) return;
    context->cpu_profile = cpu_profile;
    context->prefetch_capacity = cpu_profile == CORE_MACHINE_CPU_PROFILE_8088 ?
        4u : 15u;
    context->fpu_profile = fpu_profile;
    context->cpu_80386_cr_mov_ignores_mod = cpu_80386_cr_mov_ignores_mod;
}

void core_machine_cpu_execution_context_bind_fpu(
    core_machine_cpu_execution_context *context, core_machine_fpu *fpu)
{
    if (context != LIB_NULL) context->fpu = fpu;
}

void core_machine_cpu_execution_context_bind_external_cycle_provider(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_external_cycle_provider provider, void *provider_context)
{
    if (context == LIB_NULL) return;
    context->external_cycle_provider = provider;
    context->external_cycle_context = provider_context;
}

void core_machine_cpu_execution_context_bind_firmware_interrupt_provider(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_firmware_interrupt_provider provider, void *provider_context)
{
    if (context == LIB_NULL) return;
    context->firmware_interrupt_provider = provider;
    context->firmware_interrupt_context = provider_context;
}

void core_machine_cpu_execution_context_bind_transaction(
    core_machine_cpu_execution_context *context,
    core_machine_transaction_state *transaction)
{
    if (context != LIB_NULL) context->transaction = transaction;
}

const char *core_machine_cpu_profile_name(core_machine_cpu_profile profile)
{
    switch (profile) {
    case CORE_MACHINE_CPU_PROFILE_8086: return "8086";
    case CORE_MACHINE_CPU_PROFILE_8088: return "8088";
    case CORE_MACHINE_CPU_PROFILE_80186: return "80186";
    case CORE_MACHINE_CPU_PROFILE_80286: return "80286";
    case CORE_MACHINE_CPU_PROFILE_80386: return "80386";
    case CORE_MACHINE_CPU_PROFILE_DEFAULT: return "default";
    }
    return "invalid";
}

void core_machine_cpu_execution_context_bind_pic(
    core_machine_cpu_execution_context *context, t_pic *master,
    t_pic *slave)
{
    if (context == LIB_NULL) return;
    context->pic_master = master;
    context->pic_slave = slave;
}

void core_machine_cpu_execution_context_bind_diagnostic_provider(
    core_machine_cpu_execution_context *context,
    const core_machine_cpu_execution_diagnostic_provider *provider,
    void *provider_context)
{
    if (context == LIB_NULL) return;
    context->diagnostic_provider = provider;
    context->diagnostic_context = provider_context;
}

void core_machine_cpu_state_initialize(
    core_machine_cpu_execution_context *context) {
    if (context == LIB_NULL || context->cpu == LIB_NULL ||
        context->instructions == LIB_NULL) return;
    if (context != LIB_NULL) {
        context->stop_requested = LIB_FALSE;
        context->debug_pause_requested = LIB_FALSE;
        context->reset_requested = LIB_FALSE;
        context->shutdown_requested = LIB_FALSE;
        context->debug_trap_pending = LIB_FALSE;
        context->debug_tf_before = LIB_FALSE;
        context->debug_rf_before = LIB_FALSE;
        context->debug_trap_cause = 0u;
        context->prefetch_count = 0u;
        context->prefetch_capacity = context->cpu_profile ==
            CORE_MACHINE_CPU_PROFILE_8088 ? 4u : 15u;
        context->prefetch_valid = LIB_FALSE;
        context->prefetch_expected_valid = LIB_FALSE;
        context->prefetch_reservation_valid = LIB_FALSE;
        context->prefetch_reservation_linear = 0u;
        context->prefetch_reservation_count = 0u;
    }
    core_machine_cpu_execution_initialize(context);
}
void core_machine_cpu_state_reset(core_machine_cpu_execution_context *context) {
    if (context == LIB_NULL || context->cpu == LIB_NULL ||
        context->instructions == LIB_NULL) return;
    lib_memory_set((void *)context->cpu, 0u, sizeof(t_cpu));
    if (context != LIB_NULL) {
        context->stop_requested = LIB_FALSE;
        context->debug_pause_requested = LIB_FALSE;
        context->reset_requested = LIB_FALSE;
        context->shutdown_requested = LIB_FALSE;
        context->prefetch_count = 0u;
        context->prefetch_capacity = context->cpu_profile ==
            CORE_MACHINE_CPU_PROFILE_8088 ? 4u : 15u;
        context->prefetch_valid = LIB_FALSE;
        context->prefetch_expected_valid = LIB_FALSE;
        context->prefetch_reservation_valid = LIB_FALSE;
        context->prefetch_reservation_linear = 0u;
        context->prefetch_reservation_count = 0u;
    }

    cpu_state.data.eip = 0x0000fff0;
    cpu_state.data.eflags = 0x00000002;
    /* Intel 80386 PRM 10.1 defines DH=3 after RESET# for the 386DX.
     * The selected zero revision keeps the documented device identifier
     * without inventing a board-specific stepping value. */
    if (context->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386)
        cpu_state.data.edx = 0x00000300u;

    cpu_state.data.cs.base = core_machine_cpu_reset_code_base(context->cpu_profile);
    cpu_state.data.cs.dpl = 0u;
    cpu_state.data.cs.limit = LIB_UINT32_MAX;
    cpu_state.data.cs.seg.accessed = LIB_TRUE;
    cpu_state.data.cs.seg.executable = LIB_TRUE;
    cpu_state.data.cs.seg.exec.conform = LIB_FALSE;
    cpu_state.data.cs.seg.exec.defsize = LIB_FALSE;
    cpu_state.data.cs.seg.exec.readable = LIB_TRUE;
    cpu_state.data.cs.selector = 0xf000;
    cpu_state.data.cs.sregtype = SREG_CODE;
    cpu_state.data.cs.flagValid = LIB_TRUE;

    cpu_state.data.ss.base = 0u;
    cpu_state.data.ss.dpl = 0u;
    cpu_state.data.ss.limit = 0xffffu;
    cpu_state.data.ss.seg.accessed = LIB_TRUE;
    cpu_state.data.ss.seg.executable = LIB_FALSE;
    cpu_state.data.ss.seg.data.big = LIB_FALSE;
    cpu_state.data.ss.seg.data.expdown = LIB_FALSE;
    cpu_state.data.ss.seg.data.writable = LIB_TRUE;
    cpu_state.data.ss.selector = 0u;
    cpu_state.data.ss.sregtype = SREG_STACK;
    cpu_state.data.ss.flagValid = LIB_TRUE;

    cpu_state.data.ds.base = 0u;
    cpu_state.data.ds.dpl = 0u;
    cpu_state.data.ds.limit = 0xffffu;
    cpu_state.data.ds.seg.accessed = LIB_TRUE;
    cpu_state.data.ss.seg.executable = LIB_FALSE;
    cpu_state.data.ds.seg.data.big = LIB_FALSE;
    cpu_state.data.ds.seg.data.expdown = LIB_FALSE;
    cpu_state.data.ds.seg.data.writable = LIB_TRUE;
    cpu_state.data.ds.selector = 0u;
    cpu_state.data.ds.sregtype = SREG_DATA;
    cpu_state.data.ds.flagValid = LIB_TRUE;
    cpu_state.data.gs = cpu_state.data.fs = cpu_state.data.es = cpu_state.data.ds;

    cpu_state.data.ldtr.base = 0u;
    cpu_state.data.ldtr.dpl = 0u;
    cpu_state.data.ldtr.limit = 0xffffu;
    cpu_state.data.ldtr.selector = 0u;
    cpu_state.data.ldtr.sregtype = SREG_LDTR;
    cpu_state.data.ldtr.sys.type = VCPU_DESC_SYS_TYPE_LDT;
    cpu_state.data.ldtr.flagValid = LIB_TRUE;

    cpu_state.data.tr.base = 0u;
    cpu_state.data.tr.dpl = 0u;
    cpu_state.data.tr.limit = 0xffffu;
    cpu_state.data.tr.selector = 0u;
    cpu_state.data.tr.sregtype = SREG_TR;
    cpu_state.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_16_AVL;
    cpu_state.data.tr.flagValid = LIB_TRUE;

    cpu_state.data.idtr.base = 0u;
    cpu_state.data.idtr.limit = 0x03ff;
    cpu_state.data.idtr.sregtype = SREG_IDTR;
    cpu_state.data.idtr.flagValid = LIB_TRUE;

    cpu_state.data.gdtr.base = 0u;
    cpu_state.data.gdtr.limit = 0xffffu;
    cpu_state.data.gdtr.sregtype = SREG_GDTR;
    cpu_state.data.gdtr.flagValid = LIB_TRUE;

    core_machine_cpu_execution_reset(context);

}

void core_machine_cpu_execution_reserve_prefetch(
    core_machine_cpu_execution_context *context)
{
    lib_u32 offset;

    if (context == LIB_NULL || context->cpu == LIB_NULL ||
        (context->cpu->data.cr0 & VCPU_CR0_PG) ||
        context->prefetch_reservation_valid || !context->prefetch_valid ||
        context->prefetch_expected_linear < context->prefetch_linear ||
        cpu_state.data.eip > cpu_state.data.cs.limit) return;
    if (context->cpu_profile != CORE_MACHINE_CPU_PROFILE_8088) {
        context->prefetch_reservation_linear = context->prefetch_expected_linear;
        context->prefetch_reservation_count = context->prefetch_count;
        context->prefetch_reservation_valid = LIB_TRUE;
        return;
    }
    offset = context->prefetch_expected_linear - context->prefetch_linear;
    if (offset >= context->prefetch_count) return;
    if (offset != 0u) {
        context->prefetch_count = (lib_u8)(context->prefetch_count - offset);
        lib_memory_move(context->prefetch_bytes, context->prefetch_bytes + offset,
            context->prefetch_count);
        context->prefetch_linear = context->prefetch_expected_linear;
    }
    if (context->prefetch_count >= context->prefetch_capacity) return;
    context->prefetch_reservation_linear = context->prefetch_linear +
        context->prefetch_count;
    context->prefetch_reservation_count = 1u;
    context->prefetch_reservation_valid = LIB_TRUE;
}

void core_machine_cpu_execution_advance_prefetch_reservation(
    core_machine_cpu_execution_context *context)
{
    lib_u8 byte;

    if (context == LIB_NULL || !context->prefetch_reservation_valid) return;
    if (context->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
        context->memory_access_provenance =
            CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH;
        if (!core_machine_cpu_execution_read_linear(context,
                context->prefetch_reservation_linear, (lib_uptr)&byte, 1u) &&
            context->prefetch_count < context->prefetch_capacity) {
            context->prefetch_bytes[context->prefetch_count++] = byte;
        }
        context->memory_access_provenance = CORE_MACHINE_CPU_MEMORY_ACCESS_DATA;
    }
    context->prefetch_reservation_valid = LIB_FALSE;
    context->prefetch_reservation_linear = 0u;
    context->prefetch_reservation_count = 0u;
}
void core_machine_cpu_execution_invalidate_prefetch(
    core_machine_cpu_execution_context *context)
{
    if (context == LIB_NULL) return;
    context->prefetch_count = 0u;
    context->prefetch_valid = LIB_FALSE;
    context->prefetch_expected_valid = LIB_FALSE;
    context->prefetch_reservation_valid = LIB_FALSE;
    context->prefetch_reservation_linear = 0u;
    context->prefetch_reservation_count = 0u;
}

void core_machine_cpu_execution_request_stop(
    core_machine_cpu_execution_context *context)
{
    if (context != LIB_NULL) context->stop_requested = LIB_TRUE;
}
lib_u8 core_machine_cpu_execution_consume_stop_request(
    core_machine_cpu_execution_context *context)
{
    lib_u8 requested = context != LIB_NULL && context->stop_requested;
    if (context != LIB_NULL) context->stop_requested = LIB_FALSE;
    return requested;
}
void core_machine_cpu_execution_request_debug_pause(
    core_machine_cpu_execution_context *context)
{
    if (context != LIB_NULL) context->debug_pause_requested = LIB_TRUE;
}

lib_u8 core_machine_cpu_execution_consume_debug_pause_request(
    core_machine_cpu_execution_context *context)
{
    lib_u8 requested = context != LIB_NULL && context->debug_pause_requested;

    if (context != LIB_NULL) context->debug_pause_requested = LIB_FALSE;
    return requested;
}
void core_machine_cpu_execution_request_reset(
    core_machine_cpu_execution_context *context)
{
    if (context != LIB_NULL) context->reset_requested = LIB_TRUE;
}
lib_u8 core_machine_cpu_execution_consume_reset_request(
    core_machine_cpu_execution_context *context)
{
    lib_u8 requested = context != LIB_NULL && context->reset_requested;
    if (context != LIB_NULL) context->reset_requested = LIB_FALSE;
    return requested;
}
void core_machine_cpu_execution_request_shutdown(
    core_machine_cpu_execution_context *context)
{
    if (context != LIB_NULL) context->shutdown_requested = LIB_TRUE;
}
lib_u8 core_machine_cpu_execution_consume_shutdown_request(
    core_machine_cpu_execution_context *context)
{
    lib_u8 requested = context != LIB_NULL && context->shutdown_requested;
    if (context != LIB_NULL) context->shutdown_requested = LIB_FALSE;
    return requested;
}

lib_i32 core_machine_cpu_read_linear(core_machine_cpu_execution_context *context, lib_u32 linear, void *out_data, lib_u8 size)
{
    return core_machine_cpu_execution_read_linear(context, linear,
        (lib_uptr)out_data, size);
}

lib_i32 core_machine_cpu_write_linear(core_machine_cpu_execution_context *context,
    lib_u32 linear, const void *in_data, lib_u8 size)
{
    return core_machine_cpu_execution_write_linear(context, linear,
        (lib_uptr)in_data, size);
}

lib_i32 core_machine_cpu_get_code_default_size(const core_machine_cpu_execution_context *context)
{
    return cpu_state.data.cs.seg.exec.defsize;
}

lib_u32 core_machine_cpu_get_code_base(const core_machine_cpu_execution_context *context)
{
    return cpu_state.data.cs.base;
}

void core_machine_cpu_set_watchpoint(core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind, lib_u32 linear)
{
    switch (kind) {
    case CORE_MACHINE_CPU_WATCH_READ:
        instruction_state.data.wrLinear = linear;
        instruction_state.data.flagWR = LIB_TRUE;
        break;
    case CORE_MACHINE_CPU_WATCH_WRITE:
        instruction_state.data.wwLinear = linear;
        instruction_state.data.flagWW = LIB_TRUE;
        break;
    case CORE_MACHINE_CPU_WATCH_EXECUTE:
        instruction_state.data.weLinear = linear;
        instruction_state.data.flagWE = LIB_TRUE;
        break;
    }
}

void core_machine_cpu_clear_watchpoint(core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind)
{
    switch (kind) {
    case CORE_MACHINE_CPU_WATCH_READ:
        instruction_state.data.flagWR = LIB_FALSE;
        break;
    case CORE_MACHINE_CPU_WATCH_WRITE:
        instruction_state.data.flagWW = LIB_FALSE;
        break;
    case CORE_MACHINE_CPU_WATCH_EXECUTE:
        instruction_state.data.flagWE = LIB_FALSE;
        break;
    }
}

void core_machine_cpu_get_watchpoint(const core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind, lib_u8 *out_enabled,
    lib_u32 *out_linear)
{
    if (out_enabled == LIB_NULL || out_linear == LIB_NULL) return;
    *out_enabled = LIB_FALSE;
    *out_linear = 0u;
    switch (kind) {
    case CORE_MACHINE_CPU_WATCH_READ:
        *out_enabled = instruction_state.data.flagWR;
        *out_linear = instruction_state.data.wrLinear;
        break;
    case CORE_MACHINE_CPU_WATCH_WRITE:
        *out_enabled = instruction_state.data.flagWW;
        *out_linear = instruction_state.data.wwLinear;
        break;
    case CORE_MACHINE_CPU_WATCH_EXECUTE:
        *out_enabled = instruction_state.data.flagWE;
        *out_linear = instruction_state.data.weLinear;
        break;
    }
}

