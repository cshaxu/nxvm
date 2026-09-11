/* Copyright 2012-2014 Neko. */

/* VCPU defines the Central Processing Unit. */

#include "type.h"

#include "core/machine/cpu_instructions.h"

#include "core/machine/transaction.h"

#include "core/machine/cpu.h"

#define cpu_state (*context->cpu)
#define instruction_state (*context->instructions)

static type_unsigned_32 core_machine_cpu_reset_code_base(
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

C_VOID core_machine_cpu_execution_context_initialize(
    core_machine_cpu_execution_context *context, t_cpu *cpu,
    t_cpuins *instructions, t_ram *memory, t_port *port)
{
    if (context == STD_NULL) return;
    context->cpu = cpu;
    context->instructions = instructions;
    context->memory = memory;
    context->port = port;
    context->transaction = STD_NULL;
    context->pic_master = STD_NULL;
    context->pic_slave = STD_NULL;
    if (context->trace == STD_NULL) {
        context->trace = (type_trace *)STD_MALLOC(sizeof(*context->trace));
    }
    if (context->trace != STD_NULL) {
        STD_MEMSET((C_VOID *)context->trace, TYPE_ZERO_8, sizeof(*context->trace));
    }
    context->diagnostic_provider = STD_NULL;
    context->diagnostic_context = STD_NULL;
    context->external_cycle_provider = STD_NULL;
    context->external_cycle_context = STD_NULL;
    context->firmware_interrupt_provider = STD_NULL;
    context->firmware_interrupt_context = STD_NULL;
    context->stop_requested = TYPE_FALSE;
    context->reset_requested = TYPE_FALSE;
    context->debug_trap_pending = TYPE_FALSE;
    context->debug_tf_before = TYPE_FALSE;
    context->debug_rf_before = TYPE_FALSE;
    context->debug_trap_cause = TYPE_ZERO_32;
    context->preview_mode = TYPE_FALSE;
    context->memory_access_provenance = CORE_MACHINE_CPU_MEMORY_ACCESS_DATA;
    context->prefetch_count = 0u;
    context->prefetch_capacity = 15u;
    context->prefetch_valid = TYPE_FALSE;
    context->prefetch_expected_valid = TYPE_FALSE;
    context->prefetch_reservation_valid = TYPE_FALSE;
    context->prefetch_reservation_linear = 0u;
    context->prefetch_reservation_count = 0u;
    context->cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    context->fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE;
    context->cpu_80386_cr_mov_ignores_mod = TYPE_FALSE;
    context->fpu = STD_NULL;
}

C_VOID core_machine_cpu_execution_context_bind_profiles(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_profile cpu_profile,
    core_machine_fpu_profile fpu_profile,
    type_bool cpu_80386_cr_mov_ignores_mod)
{
    if (context == STD_NULL) return;
    context->cpu_profile = cpu_profile;
    context->prefetch_capacity = cpu_profile == CORE_MACHINE_CPU_PROFILE_8088 ?
        4u : 15u;
    context->fpu_profile = fpu_profile;
    context->cpu_80386_cr_mov_ignores_mod = cpu_80386_cr_mov_ignores_mod;
}

C_VOID core_machine_cpu_execution_context_bind_fpu(
    core_machine_cpu_execution_context *context, core_machine_fpu *fpu)
{
    if (context != STD_NULL) context->fpu = fpu;
}

C_VOID core_machine_cpu_execution_context_bind_external_cycle_provider(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_external_cycle_provider provider, C_VOID *provider_context)
{
    if (context == STD_NULL) return;
    context->external_cycle_provider = provider;
    context->external_cycle_context = provider_context;
}

C_VOID core_machine_cpu_execution_context_bind_firmware_interrupt_provider(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_firmware_interrupt_provider provider, C_VOID *provider_context)
{
    if (context == STD_NULL) return;
    context->firmware_interrupt_provider = provider;
    context->firmware_interrupt_context = provider_context;
}

C_VOID core_machine_cpu_execution_context_bind_transaction(
    core_machine_cpu_execution_context *context,
    core_machine_transaction_state *transaction)
{
    if (context != STD_NULL) context->transaction = transaction;
}

const C_CHAR *core_machine_cpu_profile_name(core_machine_cpu_profile profile)
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

C_VOID core_machine_cpu_execution_context_bind_pic(
    core_machine_cpu_execution_context *context, t_pic *master,
    t_pic *slave)
{
    if (context == STD_NULL) return;
    context->pic_master = master;
    context->pic_slave = slave;
}

C_VOID core_machine_cpu_execution_context_bind_diagnostic_provider(
    core_machine_cpu_execution_context *context,
    const core_machine_cpu_execution_diagnostic_provider *provider,
    C_VOID *provider_context)
{
    if (context == STD_NULL) return;
    context->diagnostic_provider = provider;
    context->diagnostic_context = provider_context;
}

C_VOID core_machine_cpu_state_initialize(
    core_machine_cpu_execution_context *context) {
    if (context == STD_NULL || context->cpu == STD_NULL ||
        context->instructions == STD_NULL) return;
    if (context != STD_NULL) {
        context->stop_requested = TYPE_FALSE;
        context->reset_requested = TYPE_FALSE;
        context->shutdown_requested = TYPE_FALSE;
        context->debug_trap_pending = TYPE_FALSE;
        context->debug_tf_before = TYPE_FALSE;
        context->debug_rf_before = TYPE_FALSE;
        context->debug_trap_cause = TYPE_ZERO_32;
        context->prefetch_count = 0u;
        context->prefetch_capacity = context->cpu_profile ==
            CORE_MACHINE_CPU_PROFILE_8088 ? 4u : 15u;
        context->prefetch_valid = TYPE_FALSE;
        context->prefetch_expected_valid = TYPE_FALSE;
        context->prefetch_reservation_valid = TYPE_FALSE;
        context->prefetch_reservation_linear = 0u;
        context->prefetch_reservation_count = 0u;
    }
    core_machine_cpu_execution_initialize(context);
}
C_VOID core_machine_cpu_state_reset(core_machine_cpu_execution_context *context) {
    if (context == STD_NULL || context->cpu == STD_NULL ||
        context->instructions == STD_NULL) return;
    STD_MEMSET((C_VOID *)context->cpu, TYPE_ZERO_8, sizeof(t_cpu));
    if (context != STD_NULL) {
        context->stop_requested = TYPE_FALSE;
        context->reset_requested = TYPE_FALSE;
        context->shutdown_requested = TYPE_FALSE;
        context->prefetch_count = 0u;
        context->prefetch_capacity = context->cpu_profile ==
            CORE_MACHINE_CPU_PROFILE_8088 ? 4u : 15u;
        context->prefetch_valid = TYPE_FALSE;
        context->prefetch_expected_valid = TYPE_FALSE;
        context->prefetch_reservation_valid = TYPE_FALSE;
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
    cpu_state.data.cs.dpl = TYPE_ZERO_4;
    cpu_state.data.cs.limit = TYPE_MAX_UNSIGNED_32;
    cpu_state.data.cs.seg.accessed = TYPE_TRUE;
    cpu_state.data.cs.seg.executable = TYPE_TRUE;
    cpu_state.data.cs.seg.exec.conform = TYPE_FALSE;
    cpu_state.data.cs.seg.exec.defsize = TYPE_FALSE;
    cpu_state.data.cs.seg.exec.readable = TYPE_TRUE;
    cpu_state.data.cs.selector = 0xf000;
    cpu_state.data.cs.sregtype = SREG_CODE;
    cpu_state.data.cs.flagValid = TYPE_TRUE;

    cpu_state.data.ss.base = TYPE_ZERO_32;
    cpu_state.data.ss.dpl = TYPE_ZERO_4;
    cpu_state.data.ss.limit = TYPE_MAX_UNSIGNED_16;
    cpu_state.data.ss.seg.accessed = TYPE_TRUE;
    cpu_state.data.ss.seg.executable = TYPE_FALSE;
    cpu_state.data.ss.seg.data.big = TYPE_FALSE;
    cpu_state.data.ss.seg.data.expdown = TYPE_FALSE;
    cpu_state.data.ss.seg.data.writable = TYPE_TRUE;
    cpu_state.data.ss.selector = TYPE_ZERO_16;
    cpu_state.data.ss.sregtype = SREG_STACK;
    cpu_state.data.ss.flagValid = TYPE_TRUE;

    cpu_state.data.ds.base = TYPE_ZERO_32;
    cpu_state.data.ds.dpl = TYPE_ZERO_4;
    cpu_state.data.ds.limit = TYPE_MAX_UNSIGNED_16;
    cpu_state.data.ds.seg.accessed = TYPE_TRUE;
    cpu_state.data.ss.seg.executable = TYPE_FALSE;
    cpu_state.data.ds.seg.data.big = TYPE_FALSE;
    cpu_state.data.ds.seg.data.expdown = TYPE_FALSE;
    cpu_state.data.ds.seg.data.writable = TYPE_TRUE;
    cpu_state.data.ds.selector = TYPE_ZERO_16;
    cpu_state.data.ds.sregtype = SREG_DATA;
    cpu_state.data.ds.flagValid = TYPE_TRUE;
    cpu_state.data.gs = cpu_state.data.fs = cpu_state.data.es = cpu_state.data.ds;

    cpu_state.data.ldtr.base = TYPE_ZERO_32;
    cpu_state.data.ldtr.dpl = TYPE_ZERO_4;
    cpu_state.data.ldtr.limit = TYPE_MAX_UNSIGNED_16;
    cpu_state.data.ldtr.selector = TYPE_ZERO_16;
    cpu_state.data.ldtr.sregtype = SREG_LDTR;
    cpu_state.data.ldtr.sys.type = VCPU_DESC_SYS_TYPE_LDT;
    cpu_state.data.ldtr.flagValid = TYPE_TRUE;

    cpu_state.data.tr.base = TYPE_ZERO_32;
    cpu_state.data.tr.dpl = TYPE_ZERO_4;
    cpu_state.data.tr.limit = TYPE_MAX_UNSIGNED_16;
    cpu_state.data.tr.selector = TYPE_ZERO_16;
    cpu_state.data.tr.sregtype = SREG_TR;
    cpu_state.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_16_AVL;
    cpu_state.data.tr.flagValid = TYPE_TRUE;

    cpu_state.data.idtr.base = TYPE_ZERO_32;
    cpu_state.data.idtr.limit = 0x03ff;
    cpu_state.data.idtr.sregtype = SREG_IDTR;
    cpu_state.data.idtr.flagValid = TYPE_TRUE;

    cpu_state.data.gdtr.base = TYPE_ZERO_32;
    cpu_state.data.gdtr.limit = TYPE_MAX_UNSIGNED_16;
    cpu_state.data.gdtr.sregtype = SREG_GDTR;
    cpu_state.data.gdtr.flagValid = TYPE_TRUE;

    core_machine_cpu_execution_reset(context);

}

C_VOID core_machine_cpu_execution_reserve_prefetch(
    core_machine_cpu_execution_context *context)
{
    type_unsigned_32 offset;

    if (context == STD_NULL || context->cpu == STD_NULL ||
        (context->cpu->data.cr0 & VCPU_CR0_PG) ||
        context->prefetch_reservation_valid || !context->prefetch_valid ||
        context->prefetch_expected_linear < context->prefetch_linear ||
        cpu_state.data.eip > cpu_state.data.cs.limit) return;
    if (context->cpu_profile != CORE_MACHINE_CPU_PROFILE_8088) {
        context->prefetch_reservation_linear = context->prefetch_expected_linear;
        context->prefetch_reservation_count = context->prefetch_count;
        context->prefetch_reservation_valid = TYPE_TRUE;
        return;
    }
    offset = context->prefetch_expected_linear - context->prefetch_linear;
    if (offset >= context->prefetch_count) return;
    if (offset != 0u) {
        context->prefetch_count = (type_unsigned_8)(context->prefetch_count - offset);
        STD_MEMMOVE(context->prefetch_bytes, context->prefetch_bytes + offset,
            context->prefetch_count);
        context->prefetch_linear = context->prefetch_expected_linear;
    }
    if (context->prefetch_count >= context->prefetch_capacity) return;
    context->prefetch_reservation_linear = context->prefetch_linear +
        context->prefetch_count;
    context->prefetch_reservation_count = 1u;
    context->prefetch_reservation_valid = TYPE_TRUE;
}

C_VOID core_machine_cpu_execution_advance_prefetch_reservation(
    core_machine_cpu_execution_context *context)
{
    type_unsigned_8 byte;

    if (context == STD_NULL || !context->prefetch_reservation_valid) return;
    if (context->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
        context->memory_access_provenance =
            CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH;
        if (!core_machine_cpu_execution_read_linear(context,
                context->prefetch_reservation_linear, (type_virtual_address)&byte, 1u) &&
            context->prefetch_count < context->prefetch_capacity) {
            context->prefetch_bytes[context->prefetch_count++] = byte;
        }
        context->memory_access_provenance = CORE_MACHINE_CPU_MEMORY_ACCESS_DATA;
    }
    context->prefetch_reservation_valid = TYPE_FALSE;
    context->prefetch_reservation_linear = 0u;
    context->prefetch_reservation_count = 0u;
}
C_VOID core_machine_cpu_execution_invalidate_prefetch(
    core_machine_cpu_execution_context *context)
{
    if (context == STD_NULL) return;
    context->prefetch_count = 0u;
    context->prefetch_valid = TYPE_FALSE;
    context->prefetch_expected_valid = TYPE_FALSE;
    context->prefetch_reservation_valid = TYPE_FALSE;
    context->prefetch_reservation_linear = 0u;
    context->prefetch_reservation_count = 0u;
}

C_VOID core_machine_cpu_execution_request_stop(
    core_machine_cpu_execution_context *context)
{
    if (context != STD_NULL) context->stop_requested = TYPE_TRUE;
}
 type_bool core_machine_cpu_execution_consume_stop_request(
    core_machine_cpu_execution_context *context)
{
    type_bool requested = context != STD_NULL && context->stop_requested;
    if (context != STD_NULL) context->stop_requested = TYPE_FALSE;
    return requested;
}
C_VOID core_machine_cpu_execution_request_reset(
    core_machine_cpu_execution_context *context)
{
    if (context != STD_NULL) context->reset_requested = TYPE_TRUE;
}
type_bool core_machine_cpu_execution_consume_reset_request(
    core_machine_cpu_execution_context *context)
{
    type_bool requested = context != STD_NULL && context->reset_requested;
    if (context != STD_NULL) context->reset_requested = TYPE_FALSE;
    return requested;
}
C_VOID core_machine_cpu_execution_request_shutdown(
    core_machine_cpu_execution_context *context)
{
    if (context != STD_NULL) context->shutdown_requested = TYPE_TRUE;
}
type_bool core_machine_cpu_execution_consume_shutdown_request(
    core_machine_cpu_execution_context *context)
{
    type_bool requested = context != STD_NULL && context->shutdown_requested;
    if (context != STD_NULL) context->shutdown_requested = TYPE_FALSE;
    return requested;
}

C_INT core_machine_cpu_read_linear(core_machine_cpu_execution_context *context, type_unsigned_32 linear, C_VOID *out_data, type_unsigned_8 size)
{
    return core_machine_cpu_execution_read_linear(context, linear,
        (type_virtual_address)out_data, size);
}

C_INT core_machine_cpu_write_linear(core_machine_cpu_execution_context *context,
    type_unsigned_32 linear, const C_VOID *in_data, type_unsigned_8 size)
{
    return core_machine_cpu_execution_write_linear(context, linear,
        (type_virtual_address)in_data, size);
}

C_INT core_machine_cpu_load_segment(core_machine_cpu_execution_context *context,
    core_machine_cpu_segment segment, type_unsigned_16 selector)
{
    switch (segment) {
    case CORE_MACHINE_CPU_SEGMENT_ES:
        return core_machine_cpu_execution_load_segment(context,
            &cpu_state.data.es, selector);
    case CORE_MACHINE_CPU_SEGMENT_CS:
        return core_machine_cpu_execution_load_segment(context,
            &cpu_state.data.cs, selector);
    case CORE_MACHINE_CPU_SEGMENT_SS:
        return core_machine_cpu_execution_load_segment(context,
            &cpu_state.data.ss, selector);
    case CORE_MACHINE_CPU_SEGMENT_DS:
        return core_machine_cpu_execution_load_segment(context,
            &cpu_state.data.ds, selector);
    case CORE_MACHINE_CPU_SEGMENT_FS:
        return core_machine_cpu_execution_load_segment(context,
            &cpu_state.data.fs, selector);
    case CORE_MACHINE_CPU_SEGMENT_GS:
        return core_machine_cpu_execution_load_segment(context,
            &cpu_state.data.gs, selector);
    }
    return 1;
}

C_INT core_machine_cpu_get_code_default_size(const core_machine_cpu_execution_context *context)
{
    return cpu_state.data.cs.seg.exec.defsize;
}

type_unsigned_32 core_machine_cpu_get_code_base(const core_machine_cpu_execution_context *context)
{
    return cpu_state.data.cs.base;
}

C_VOID core_machine_cpu_set_watchpoint(core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind, type_unsigned_32 linear)
{
    switch (kind) {
    case CORE_MACHINE_CPU_WATCH_READ:
        instruction_state.data.wrLinear = linear;
        instruction_state.data.flagWR = TYPE_TRUE;
        break;
    case CORE_MACHINE_CPU_WATCH_WRITE:
        instruction_state.data.wwLinear = linear;
        instruction_state.data.flagWW = TYPE_TRUE;
        break;
    case CORE_MACHINE_CPU_WATCH_EXECUTE:
        instruction_state.data.weLinear = linear;
        instruction_state.data.flagWE = TYPE_TRUE;
        break;
    }
}

C_VOID core_machine_cpu_clear_watchpoint(core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind)
{
    switch (kind) {
    case CORE_MACHINE_CPU_WATCH_READ:
        instruction_state.data.flagWR = TYPE_FALSE;
        break;
    case CORE_MACHINE_CPU_WATCH_WRITE:
        instruction_state.data.flagWW = TYPE_FALSE;
        break;
    case CORE_MACHINE_CPU_WATCH_EXECUTE:
        instruction_state.data.flagWE = TYPE_FALSE;
        break;
    }
}

C_VOID core_machine_cpu_get_watchpoint(const core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind, type_bool *out_enabled,
    type_unsigned_32 *out_linear)
{
    if (out_enabled == STD_NULL || out_linear == STD_NULL) return;
    *out_enabled = TYPE_FALSE;
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

