#include "x86-core/cpu/cpu_instructions.h"
#include "x86-core/cpu/cpu_timing.h"

/* Intel 273021-001 3.1.4--3.1.6, with the explicit atomic L2 service model.
 * These slots contain latency/ordering only. Functional writes keep their
 * sole synchronous provider and are never replayed during advancement. */
static lib_bool cpu_bus_observes(const x86_core_cpu_execution_context *context)
{
    return context != LIB_NULL && context->cpu != LIB_NULL &&
        context->instructions != LIB_NULL &&
        context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        context->timing_bus_active && !context->preview_mode &&
        !context->bus_timing.invalid;
}

static lib_bool cpu_bus_add(x86_core_cpu_execution_context *context,
    lib_u64 *total, lib_u64 ticks)
{
    if (ticks > LIB_UINT64_MAX - *total) {
        context->bus_timing.invalid = LIB_TRUE;
        return LIB_FALSE;
    }
    *total += ticks;
    return LIB_TRUE;
}

static void cpu_bus_service(cpu_bus_timing *bus, lib_u64 ticks)
{
    while (ticks != 0u && bus->count != 0u) {
        cpu_write_buffer *write = &bus->writes[bus->head];

        if (ticks < write->ticks) {
            write->ticks -= ticks;
            bus->write_active = LIB_TRUE;
            return;
        }
        ticks -= write->ticks;
        *write = (cpu_write_buffer){0};
        bus->head = (bus->head + 1u) % X86_CPU_WRITE_BUFFER_CAPACITY;
        --bus->count;
        bus->write_active = LIB_FALSE;
    }
}

static void cpu_bus_account(x86_core_cpu_execution_context *context,
    lib_u64 ticks, lib_bool service_writes)
{
    if (!cpu_bus_add(context, &context->bus_timing.service_credit, ticks) ||
        !cpu_bus_add(context, &context->instructions->data.timing_bus_service_ticks, ticks))
        return;
    if (service_writes) cpu_bus_service(&context->bus_timing, ticks);
}

static void cpu_bus_wait_head(x86_core_cpu_execution_context *context,
    lib_bool stalled)
{
    const lib_u64 ticks = context->bus_timing.writes[context->bus_timing.head].ticks;

    if (stalled && !cpu_bus_add(context,
            &context->instructions->data.timing_bus_stall_ticks, ticks)) return;
    cpu_bus_account(context, ticks, LIB_TRUE);
}

void x86_core_cpu_bus_reset(x86_core_cpu_execution_context *context)
{
    if (context != LIB_NULL)
        lib_memory_set(&context->bus_timing, 0u, sizeof(context->bus_timing));
}

void x86_core_cpu_execution_advance_bus(
    x86_core_cpu_execution_context *context, lib_u64 elapsed_ticks,
    lib_bool bus_available, lib_u64 invalidation_cycles)
{
    if (context == LIB_NULL || context->cpu == LIB_NULL || context->instructions == LIB_NULL ||
        context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX ||
        context->preview_mode || !context->clock_running || context->stop_granted ||
        context->bus_timing.invalid || elapsed_ticks == 0u) return;
    /* PRM E-2 note 9. Atomic execution supplies a conservative resource-use
     * interval, not the exact internal-cache port arbitration of a pipeline. */
    if ((context->cpu->data.cr0 & VCPU_CR0_NW) == 0u &&
        (context->instructions->data.timing_cache_used ||
         context->bus_timing.service_credit != 0u || context->bus_timing.count != 0u)) {
        if (!cpu_bus_add(context, &context->instructions->data.timing_bus_invalidation_ticks,
                invalidation_cycles < elapsed_ticks ? invalidation_cycles : elapsed_ticks))
            return;
    }
    if (!bus_available) return;
    const lib_u64 reconciled = elapsed_ticks < context->bus_timing.service_credit ?
        elapsed_ticks : context->bus_timing.service_credit;

    context->bus_timing.service_credit -= reconciled;
    cpu_bus_service(&context->bus_timing, elapsed_ticks - reconciled);
}

void x86_core_cpu_bus_drain(x86_core_cpu_execution_context *context, lib_bool stalled)
{
    if (!cpu_bus_observes(context)) return;
    while (context->bus_timing.count != 0u && !context->bus_timing.invalid)
        cpu_bus_wait_head(context, stalled);
}

void x86_core_cpu_bus_invalidate(x86_core_cpu_execution_context *context)
{
    if (context == LIB_NULL || context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX ||
        context->preview_mode) return;
    for (lib_u8 index = 0u; index < context->bus_timing.count; ++index)
        context->bus_timing.writes[(context->bus_timing.head + index) %
            X86_CPU_WRITE_BUFFER_CAPACITY].cache_hit = LIB_FALSE;
}

void x86_core_cpu_bus_prepare_read(x86_core_cpu_execution_context *context,
    lib_bool ordered)
{
    if (!cpu_bus_observes(context) || context->bus_timing.count == 0u) return;
    if (ordered || (context->cpu->data.cr0 & VCPU_CR0_CD) != 0u) {
        x86_core_cpu_bus_drain(context, LIB_TRUE);
        return;
    }
    /* A read cannot preempt a write already on the external bus. */
    if (context->bus_timing.write_active) cpu_bus_wait_head(context, LIB_TRUE);
    for (lib_u8 index = 0u; index < context->bus_timing.count; ++index) {
        if (!context->bus_timing.writes[(context->bus_timing.head + index) %
                X86_CPU_WRITE_BUFFER_CAPACITY].cache_hit) {
            x86_core_cpu_bus_drain(context, LIB_TRUE);
            return;
        }
    }
    x86_core_cpu_bus_invalidate(context);
}

void x86_core_cpu_bus_prepare_write(x86_core_cpu_execution_context *context)
{
    if (!cpu_bus_observes(context)) return;
    if ((context->cpu->data.cr0 & VCPU_CR0_CD) != 0u)
        x86_core_cpu_bus_drain(context, LIB_TRUE);
    else if (context->bus_timing.count == X86_CPU_WRITE_BUFFER_CAPACITY)
        cpu_bus_wait_head(context, LIB_TRUE);
}

void x86_core_cpu_bus_read_complete(x86_core_cpu_execution_context *context,
    lib_u32 address, lib_u8 bytes, lib_u64 wait_ticks)
{
    lib_u64 duration = 1u + ((address & 3u) + bytes + 3u) / 4u;

    if (!cpu_bus_observes(context) || bytes == 0u) return;
    if (!cpu_bus_add(context, &context->instructions->data.timing_transfer_ticks, duration))
        return;
    if (!cpu_bus_add(context, &duration, wait_ticks) ||
        !cpu_bus_add(context, &context->instructions->data.timing_bus_wait_ticks, wait_ticks))
        return;
    /* The external read owns the bus; queued writes cannot use its clocks. */
    cpu_bus_account(context, duration, LIB_FALSE);
}

void x86_core_cpu_bus_write_complete(x86_core_cpu_execution_context *context,
    lib_u32 address, lib_u8 bytes, lib_u64 wait_ticks, lib_bool cache_hit,
    lib_bool buffered)
{
    const lib_u64 transfer_ticks = 2u * (((address & 3u) + bytes + 3u) / 4u);

    if (!cpu_bus_observes(context) || bytes == 0u) return;
    if (!cpu_bus_add(context, &context->instructions->data.timing_transfer_ticks, transfer_ticks))
        return;
    if (!buffered) {
        lib_u64 duration = transfer_ticks;

        if (!cpu_bus_add(context, &duration, wait_ticks) ||
            !cpu_bus_add(context, &context->instructions->data.timing_bus_wait_ticks, wait_ticks))
            return;
        cpu_bus_account(context, duration, LIB_FALSE);
        return;
    }
    for (lib_u8 done = 0u; done < bytes && !context->bus_timing.invalid;) {
        lib_u8 part = 4u - ((address + done) & 3u);
        lib_u64 duration = 2u;

        if (part > bytes - done) part = bytes - done;
        if (context->bus_timing.count == X86_CPU_WRITE_BUFFER_CAPACITY)
            cpu_bus_wait_head(context, LIB_TRUE);
        if (!cpu_bus_add(context, &duration, done == 0u ? wait_ticks : 0u) ||
            context->bus_timing.invalid) return;
        context->bus_timing.writes[(context->bus_timing.head + context->bus_timing.count) %
            X86_CPU_WRITE_BUFFER_CAPACITY] = (cpu_write_buffer){duration, cache_hit};
        ++context->bus_timing.count;
        /* At most one 32-bit write can be accepted per issue clock. */
        cpu_bus_account(context, 1u, LIB_TRUE);
        done += part;
    }
}

void x86_core_cpu_bus_clear_observation(x86_core_cpu_execution_context *context)
{
    context->instructions->data.timing_bus_wait_ticks = 0u;
    context->instructions->data.timing_bus_stall_ticks = 0u;
    context->instructions->data.timing_bus_service_ticks = 0u;
    context->instructions->data.timing_bus_invalidation_ticks = 0u;
}

lib_bool x86_core_cpu_80486_bus_scope_cost(
    const x86_core_cpu_execution_context *context, lib_u64 baseline_ticks,
    lib_u64 *out_ticks)
{
    lib_u64 ticks = baseline_ticks;

    if (context == LIB_NULL || context->cpu == LIB_NULL || context->instructions == LIB_NULL ||
        context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX || out_ticks == LIB_NULL ||
        context->bus_timing.invalid) return LIB_FALSE;
    const t_cpuins_data *data = &context->instructions->data;

    if (data->timing_external_wait_overflow ||
        data->timing_bus_wait_ticks > LIB_UINT64_MAX - ticks) return LIB_FALSE;
    ticks += data->timing_bus_wait_ticks;
    if (data->timing_bus_stall_ticks > LIB_UINT64_MAX - ticks) return LIB_FALSE;
    ticks += data->timing_bus_stall_ticks;
    if (ticks < data->timing_bus_service_ticks) ticks = data->timing_bus_service_ticks;
    if (data->timing_bus_invalidation_ticks > LIB_UINT64_MAX - ticks) return LIB_FALSE;
    ticks += data->timing_bus_invalidation_ticks;
    *out_ticks = ticks;
    return LIB_TRUE;
}

lib_bool x86_core_cpu_80486_bus_instruction_cost(
    const x86_core_cpu_execution_context *context, lib_u64 baseline_ticks,
    lib_u64 *out_ticks, lib_bool *out_modeled)
{
    if (context == LIB_NULL || context->instructions == LIB_NULL || baseline_ticks == 0u ||
        context->stop_requested || context->instruction_fault_delivered || context->instruction_wait_stall ||
        context->instructions->data.except != 0u || context->instructions->data.oplen == 0u ||
        !x86_core_cpu_80486_bus_scope_cost(context, baseline_ticks, out_ticks)) return LIB_FALSE;
    if (out_modeled != LIB_NULL) *out_modeled = context->instructions->data.timing_bus_service_ticks != 0u ||
        context->instructions->data.timing_bus_invalidation_ticks != 0u;
    return LIB_TRUE;
}
