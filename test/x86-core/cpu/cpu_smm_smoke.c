#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

typedef struct smm_fixture {
    cpu_instruction_fixture instruction;
    lib_u32 image_base;
    lib_u32 next_image_base;
    lib_u32 writes;
    lib_u32 reads;
    lib_u32 fail_write;
    lib_u32 fail_read;
    lib_u32 assertions;
    lib_u32 releases;
    lib_u32 bad_order;
    lib_u32 port_transfers;
    lib_u32 lock_begins;
    lib_u32 lock_commits;
    lib_u32 cycle_polls;
    lib_u32 cycle_cancels;
    lib_u32 halt_cycles;
    lib_u32 shutdown_cycles;
    lib_u32 stop_cycles;
    x86_core_cpu_special_cycle held_cycle;
    lib_bool cycle_ready;
    lib_bool cycle_fail;
    lib_bool active;
    lib_bool saving;
    lib_bool partial_write;
    lib_bool interrupt;
    lib_bool io_smi;
    lib_bool io_stop_clock;
    lib_bool fail_port;
    lib_bool memory_smi;
    lib_bool memory_stop_clock;
    lib_bool locked;
} smm_fixture;

static lib_status smm_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    smm_fixture *fixture = opaque;

    if (!observe_only && address >= fixture->image_base &&
        address < fixture->image_base + 512u) {
        ++fixture->reads;
        if (!fixture->active || bytes != 4u ||
            address != fixture->image_base + 512u - fixture->reads * 4u)
            ++fixture->bad_order;
        if (fixture->reads == fixture->fail_read) return LIB_STATUS_IO_ERROR;
    }
    return cpu_instruction_read(&fixture->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch, cache_control);
}

static lib_status smm_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_u8 cache_control)
{
    smm_fixture *fixture = opaque;
    lib_status status;

    if (fixture->saving && address >= fixture->image_base && address < fixture->image_base + 512u) {
        ++fixture->writes;
        if (!fixture->active || bytes != 4u ||
            address != fixture->image_base + 512u - fixture->writes * 4u)
            ++fixture->bad_order;
        if (fixture->writes == fixture->fail_write) {
            if (fixture->partial_write)
                (void)cpu_instruction_write(&fixture->instruction, address, source,
                    1u, provenance, cache_control);
            return LIB_STATUS_IO_ERROR;
        }
        if (fixture->writes == 128u) fixture->saving = LIB_FALSE;
    }
    status = cpu_instruction_write(&fixture->instruction, address, source, bytes,
        provenance, cache_control);
    if (status == LIB_STATUS_OK && fixture->memory_smi && address == 0x200u) {
        if (x86_core_cpu_set_smi(&fixture->instruction.execution, LIB_TRUE) != LIB_STATUS_OK ||
            fixture->assertions) ++fixture->bad_order;
    }
    if (status == LIB_STATUS_OK && fixture->memory_stop_clock && address == 0x200u &&
        x86_core_cpu_set_stop_clock(&fixture->instruction.execution, LIB_TRUE) != LIB_STATUS_OK)
        ++fixture->bad_order;
    return status;
}

static void smm_active(void *opaque, lib_bool active)
{
    smm_fixture *fixture = opaque;

    if (fixture->active == active || fixture->locked) ++fixture->bad_order;
    fixture->active = active;
    if (active) {
        ++fixture->assertions;
        fixture->image_base = fixture->next_image_base;
        fixture->writes = fixture->reads = 0u;
        fixture->saving = LIB_TRUE;
    } else ++fixture->releases;
}

static lib_bool smm_interrupt(void *opaque)
{
    return ((smm_fixture *)opaque)->interrupt;
}

static lib_status smm_acknowledge(void *opaque, lib_u8 *vector)
{
    ((smm_fixture *)opaque)->interrupt = LIB_FALSE;
    *vector = 0x30u;
    return LIB_STATUS_OK;
}

static lib_status smm_port(void *opaque, lib_u16 port, lib_u8 bytes,
    lib_bool write, lib_u32 *value)
{
    smm_fixture *fixture = opaque;

    (void)port;
    (void)bytes;
    if (fixture->fail_port) return LIB_STATUS_IO_ERROR;
    if (!write) *value = 0x5au;
    ++fixture->port_transfers;
    return LIB_STATUS_OK;
}

static void smm_complete_port(void *opaque, lib_u16 port, lib_u8 bytes,
    lib_bool write)
{
    smm_fixture *fixture = opaque;

    (void)port;
    (void)bytes;
    (void)write;
    if (fixture->io_smi && x86_core_cpu_set_smi(
            &fixture->instruction.execution, LIB_TRUE) != LIB_STATUS_OK)
        ++fixture->bad_order;
    if (fixture->io_stop_clock && x86_core_cpu_set_stop_clock(
            &fixture->instruction.execution, LIB_TRUE) != LIB_STATUS_OK)
        ++fixture->bad_order;
}

static lib_status smm_begin_lock(void *opaque)
{
    smm_fixture *fixture = opaque;

    if (fixture->locked) ++fixture->bad_order;
    fixture->locked = LIB_TRUE;
    ++fixture->lock_begins;
    return LIB_STATUS_OK;
}

static void smm_complete_lock(void *opaque, lib_bool commit)
{
    smm_fixture *fixture = opaque;

    if (!fixture->locked) ++fixture->bad_order;
    fixture->locked = LIB_FALSE;
    if (commit) ++fixture->lock_commits;
}

static lib_status smm_special_cycle(void *opaque, x86_core_cpu_special_cycle cycle,
    lib_bool *acknowledged)
{
    smm_fixture *fixture = opaque;

    if (fixture->held_cycle != X86_CORE_CPU_SPECIAL_NONE && fixture->held_cycle != cycle)
        ++fixture->bad_order;
    if (fixture->locked) ++fixture->bad_order;
    fixture->held_cycle = cycle;
    ++fixture->cycle_polls;
    if (fixture->cycle_fail) return LIB_STATUS_IO_ERROR;
    *acknowledged = fixture->cycle_ready;
    if (*acknowledged) {
        if (cycle == X86_CORE_CPU_SPECIAL_HALT) ++fixture->halt_cycles;
        else if (cycle == X86_CORE_CPU_SPECIAL_SHUTDOWN) ++fixture->shutdown_cycles;
        else if (cycle == X86_CORE_CPU_SPECIAL_STOP_GRANT) ++fixture->stop_cycles;
        else ++fixture->bad_order;
        fixture->held_cycle = X86_CORE_CPU_SPECIAL_NONE;
    }
    return LIB_STATUS_OK;
}

static void smm_cancel_special_cycle(void *opaque, x86_core_cpu_special_cycle cycle)
{
    smm_fixture *fixture = opaque;

    if (fixture->held_cycle == X86_CORE_CPU_SPECIAL_NONE) return;
    if (fixture->held_cycle != cycle) ++fixture->bad_order;
    fixture->held_cycle = X86_CORE_CPU_SPECIAL_NONE;
    ++fixture->cycle_cancels;
}

static const x86_core_cpu_bus_provider smm_bus = {
    .read_memory = smm_read, .write_memory = smm_write,
    .interrupt_pending = smm_interrupt, .acknowledge_interrupt = smm_acknowledge,
    .transfer_port = smm_port, .complete_port = smm_complete_port,
    .system_management_active = smm_active,
    .begin_memory_lock = smm_begin_lock, .complete_memory_lock = smm_complete_lock,
    .special_cycle = smm_special_cycle,
    .cancel_special_cycle = smm_cancel_special_cycle
};

static void smm_prepare(smm_fixture *fixture)
{
    x86_core_cpu_execution_finalize(&fixture->instruction.execution);
    lib_memory_set(fixture, 0u, sizeof(*fixture));
    cpu_instruction_prepare_with_bus(&fixture->instruction,
        X86_CORE_CPU_PROFILE_80486SX, &smm_bus, fixture);
    fixture->image_base = fixture->next_image_base = 0x3fe00u;
    fixture->cycle_ready = LIB_TRUE;
    fixture->instruction.memory[0x38000u] = 0x0fu;
    fixture->instruction.memory[0x38001u] = 0xaau;
}

static lib_u32 smm_image_get(const smm_fixture *fixture, lib_u16 offset)
{
    const lib_u8 *bytes = fixture->instruction.memory + fixture->image_base + offset - 0x7e00u;

    return (lib_u32)bytes[0] | ((lib_u32)bytes[1] << 8u) |
        ((lib_u32)bytes[2] << 16u) | ((lib_u32)bytes[3] << 24u);
}

static void smm_image_put(smm_fixture *fixture, lib_u16 offset, lib_u32 value)
{
    lib_u8 *bytes = fixture->instruction.memory + fixture->image_base + offset - 0x7e00u;

    for (lib_u8 index = 0u; index < 4u; ++index) bytes[index] = (lib_u8)(value >> (index * 8u));
}

static lib_bool smm_enter(smm_fixture *fixture)
{
    x86_core_cpu_execution_context *context = &fixture->instruction.execution;

    if (x86_core_cpu_set_smi(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    return context->smm_active && !fixture->instruction.fault.valid &&
        !context->stop_requested && fixture->writes == 128u && !fixture->bad_order &&
        x86_core_cpu_execution_consume_instruction_fault_delivery(context) &&
        x86_core_cpu_capture_timing(context).ticks == 0u;
}

static lib_bool smm_round_trip(smm_fixture *fixture)
{
    for (lib_u8 mode = 0u; mode < 6u; ++mode)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes) {
        t_cpu before;
        t_cpu *cpu;
        x86_core_cpu_execution_context *context;

        smm_prepare(fixture);
        cpu = &fixture->instruction.cpu;
        context = &fixture->instruction.execution;
        cpu->data.eax = 0x10203040u;
        cpu->data.ecx = 0x50607080u;
        cpu->data.edx = 0x90a0b0c0u;
        cpu->data.ebx = 0xd0e0f001u;
        cpu->data.esp = 0x8000u;
        cpu->data.ebp = 0x12345678u;
        cpu->data.esi = 0x87654321u;
        cpu->data.edi = 0x11223344u;
        cpu->data.eip = 0x120u;
        cpu->data.eflags = 2u | VCPU_EFLAGS_CF | VCPU_EFLAGS_RF;
        cpu->data.cr1 = 0x45678u;
        cpu->data.cr2 = 0xfedcba98u;
        cpu->data.cr3 = 0x00071018u;
        cpu->data.cr4 = VCPU_CR4_VME;
        cpu->data.dr6 = 0xffff4ff0u;
        cpu->data.dr7 = 0x1000u;
        cpu->data.dr0 = 0xcafebabeu;
        cpu->data.tr7 = 0x13579bdfu;
        cpu->data.gdtr.base = 0x41000u;
        cpu->data.gdtr.limit = 0x1234u;
        cpu->data.idtr.base = 0x42000u;
        cpu->data.idtr.limit = 0x3456u;
        if (mode != 0u) {
            cpu->data.cr0 |= VCPU_CR0_PE;
            cpu->data.cs.dpl = mode == 5u ? 3u : mode - 1u;
            cpu->data.cs.selector = 0x18u | cpu->data.cs.dpl;
        }
        if (mode == 5u) cpu->data.eflags |= VCPU_EFLAGS_VM;
        /* No present page tables: state save must use physical cycles even
         * when the interrupted application cannot currently fetch a page. */
        if (mode == 4u) cpu->data.cr0 |= VCPU_CR0_PG;
        before = *cpu;
        if (!smm_enter(fixture) || fixture->assertions != 1u || fixture->releases ||
            cpu->data.cs.selector != 0x3000u || cpu->data.cs.base != 0x30000u ||
            cpu->data.eip != 0x8000u || cpu->data.cs.limit != LIB_UINT32_MAX ||
            cpu->data.ds.limit != LIB_UINT32_MAX || cpu->data.ss.seg.data.big ||
            cpu->data.eflags != 2u || cpu->data.cr0 !=
                (before.data.cr0 & ~(VCPU_CR0_PE | VCPU_CR0_EM | VCPU_CR0_TS | VCPU_CR0_PG)) ||
            smm_image_get(fixture, 0x7ff0u) != before.data.eip ||
            smm_image_get(fixture, 0x7fd0u) != before.data.eax ||
            smm_image_get(fixture, 0x7fd4u) != before.data.ecx ||
            smm_image_get(fixture, 0x7fd8u) != before.data.edx ||
            smm_image_get(fixture, 0x7fdcu) != before.data.ebx ||
            smm_image_get(fixture, 0x7fe0u) != before.data.esp ||
            smm_image_get(fixture, 0x7fe4u) != before.data.ebp ||
            smm_image_get(fixture, 0x7fe8u) != before.data.esi ||
            smm_image_get(fixture, 0x7fecu) != before.data.edi ||
            smm_image_get(fixture, 0x7f94u) != before.data.idtr.base ||
            smm_image_get(fixture, 0x7f88u) != before.data.gdtr.base ||
            smm_image_get(fixture, 0x7efcu) != 0x30000u) return LIB_FALSE;
        {
            lib_u8 bytes = 0u;

            if (prefixes & 1u) fixture->instruction.memory[0x38000u + bytes++] = 0x66u;
            if (prefixes & 2u) fixture->instruction.memory[0x38000u + bytes++] = 0x67u;
            fixture->instruction.memory[0x38000u + bytes++] = 0x0fu;
            fixture->instruction.memory[0x38000u + bytes] = 0xaau;
        }
        x86_core_cpu_execution_refresh(context);
        if (context->smm_active || fixture->active || fixture->reads != 128u ||
            fixture->releases != 1u || fixture->bad_order || fixture->instruction.fault.valid ||
            lib_memory_compare(cpu, &before, sizeof(before))) {
            lib_c_fprintf(lib_c_stderr, "SMM round trip mode=%u prefixes=%u\n",
                (lib_u32)mode, (lib_u32)prefixes);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool smm_guest_changes(smm_fixture *fixture)
{
    const lib_u8 handler[] = {
        0xb8u,0x34u,0x12u,                         /* mov ax,1234h */
        0x8eu,0xd8u,                               /* mov ds,ax */
        0x26u,0x66u,0x67u,0xc7u,0x05u,0xd0u,0xffu,0x03u,0x00u,0x78u,0x56u,0x34u,0x12u,
        0x66u,0xb8u,0x44u,0x33u,0x22u,0x11u,       /* mov eax,11223344h */
        0x0fu,0x23u,0xc0u,                         /* mov dr0,eax */
        0x0fu,0xaau
    };
    x86_core_cpu_execution_context *context = &fixture->instruction.execution;

    smm_prepare(fixture);
    fixture->instruction.cpu.data.eip = 0x100u;
    fixture->instruction.cpu.data.eflags |= VCPU_EFLAGS_RF;
    lib_memory_copy(fixture->instruction.memory + 0x38000u, handler, sizeof(handler));
    if (!smm_enter(fixture)) return LIB_FALSE;
    for (lib_u8 instruction = 0u; instruction < 6u; ++instruction) {
        x86_core_cpu_execution_refresh(context);
        if (fixture->instruction.fault.valid || context->stop_requested) return LIB_FALSE;
        if (instruction == 1u && (fixture->instruction.cpu.data.ds.base != 0x12340u ||
            fixture->instruction.cpu.data.ds.limit != LIB_UINT32_MAX)) return LIB_FALSE;
    }
    return !context->smm_active && fixture->instruction.cpu.data.eax == 0x12345678u &&
        fixture->instruction.instructions.data.timing_gpr_writes == 0xffu &&
        fixture->instruction.cpu.data.dr0 == 0x11223344u &&
        fixture->instruction.cpu.data.eip == 0x100u &&
        (fixture->instruction.cpu.data.eflags & VCPU_EFLAGS_RF) &&
        fixture->instruction.cpu.data.ds.base == 0u && !fixture->bad_order;
}

static lib_bool smm_relocation_and_pending(smm_fixture *fixture)
{
    x86_core_cpu_execution_context *context = &fixture->instruction.execution;

    smm_prepare(fixture);
    if (!smm_enter(fixture) || x86_core_cpu_set_smi(context, LIB_FALSE) != LIB_STATUS_OK ||
        x86_core_cpu_set_smi(context, LIB_TRUE) != LIB_STATUS_OK ||
        x86_core_cpu_set_smi(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
    smm_image_put(fixture, 0x7ef8u, 0x50000u);
    fixture->next_image_base = 0x5fe00u;
    x86_core_cpu_execution_refresh(context);
    /* RSM reenters the one queued SMI before any interrupted instruction. */
    if (!context->smm_active || context->smbase != 0x50000u ||
        fixture->assertions != 2u || fixture->releases != 1u || context->smi_pending ||
        fixture->instruction.cpu.data.cs.base != 0x50000u || fixture->writes != 128u ||
        fixture->bad_order) return LIB_FALSE;
    fixture->instruction.memory[0x58000u] = 0x0fu;
    fixture->instruction.memory[0x58001u] = 0xaau;
    x86_core_cpu_execution_refresh(context);
    return !context->smm_active && context->smbase == 0x50000u &&
        fixture->assertions == 2u && fixture->releases == 2u &&
        fixture->instruction.cpu.data.eip == 0u && !fixture->instruction.fault.valid;
}

static lib_bool smm_failures(smm_fixture *fixture)
{
    for (lib_u16 failure = 1u; failure <= 128u; ++failure) {
        x86_core_cpu_execution_context *context = &fixture->instruction.execution;
        t_cpu before;

        smm_prepare(fixture);
        fixture->fail_write = failure;
        fixture->partial_write = LIB_TRUE;
        fixture->instruction.cpu.data.cr0 = 0x60000010u;
        lib_memory_set(fixture->instruction.memory + fixture->image_base, 0xccu, 512u);
        before = fixture->instruction.cpu;
        if (x86_core_cpu_set_smi(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (!fixture->instruction.fault.valid ||
            fixture->instruction.fault.exception_mask != VCPUINS_EXCEPT_CE ||
            fixture->writes != failure || fixture->releases || !fixture->active ||
            fixture->bad_order || lib_memory_compare(&before, &fixture->instruction.cpu, sizeof(before)))
            return LIB_FALSE;
        if (failure > 1u && smm_image_get(fixture, 0x7ffcu) != before.data.cr0) return LIB_FALSE;
        const lib_u32 failed_address = fixture->image_base + 512u - failure * 4u;
        if (fixture->instruction.memory[failed_address] == 0xccu ||
            fixture->instruction.memory[failed_address + 1u] != 0xccu ||
            fixture->instruction.memory[failed_address + 2u] != 0xccu ||
            fixture->instruction.memory[failed_address + 3u] != 0xccu) return LIB_FALSE;
        x86_core_cpu_state_reset(context);
        if (fixture->active || fixture->releases != 1u) return LIB_FALSE;

        smm_prepare(fixture);
        if (!smm_enter(fixture)) return LIB_FALSE;
        before = fixture->instruction.cpu;
        fixture->fail_read = failure;
        x86_core_cpu_execution_refresh(context);
        if (!fixture->instruction.fault.valid ||
            fixture->instruction.fault.exception_mask != VCPUINS_EXCEPT_CE ||
            fixture->reads != failure || fixture->releases || !fixture->active ||
            fixture->bad_order || lib_memory_compare(&before, &fixture->instruction.cpu, sizeof(before)))
            return LIB_FALSE;
        x86_core_cpu_execution_finalize(context);
        if (fixture->active || fixture->releases != 1u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool smm_halt_restart(smm_fixture *fixture)
{
    for (lib_u8 restart = 0u; restart < 2u; ++restart) {
        x86_core_cpu_execution_context *context = &fixture->instruction.execution;

        smm_prepare(fixture);
        fixture->instruction.memory[0] = 0xf4u;
        fixture->instruction.memory[1] = 0x90u;
        x86_core_cpu_execution_refresh(context);
        if (!x86_core_cpu_is_halted(context) || !smm_enter(fixture) ||
            smm_image_get(fixture, 0x7f00u) != 0x00010000u ||
            smm_image_get(fixture, 0x7ff0u) != 1u) return LIB_FALSE;
        smm_image_put(fixture, 0x7f00u, restart ? 0x00010000u : 0u);
        x86_core_cpu_execution_refresh(context);
        if (context->smm_active || fixture->instruction.cpu.data.flagHalt ||
            fixture->instruction.cpu.data.eip != (restart ? 0u : 1u)) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (fixture->instruction.fault.valid ||
            fixture->instruction.cpu.data.flagHalt != restart ||
            fixture->instruction.cpu.data.eip != (restart ? 1u : 2u)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool smm_io_restart(smm_fixture *fixture)
{
    static const struct {lib_u8 bytes[3]; lib_u8 length;} cases[] = {
        {{0xe4u,0x80u,0u},2u}, {{0xe6u,0x80u,0u},2u},
        {{0x66u,0xedu,0u},2u}, {{0x66u,0xefu,0u},2u},
        {{0xf3u,0x6cu,0u},2u}, {{0xf3u,0x6eu,0u},2u}
    };

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        x86_core_cpu_execution_context *context = &fixture->instruction.execution;

        smm_prepare(fixture);
        lib_memory_copy(fixture->instruction.memory, cases[index].bytes, cases[index].length);
        fixture->instruction.cpu.data.ecx = 2u;
        fixture->instruction.cpu.data.esi = 0x200u;
        fixture->instruction.cpu.data.edi = 0x300u;
        fixture->instruction.cpu.data.dx = 0x80u;
        fixture->io_smi = LIB_TRUE;
        x86_core_cpu_execution_refresh(context);
        if (!context->smm_active || fixture->port_transfers != 1u ||
            !context->io_restart_valid || fixture->instruction.fault.valid) return LIB_FALSE;
        smm_image_put(fixture, 0x7f00u, 0xffu);
        fixture->io_smi = LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (context->smm_active || fixture->instruction.cpu.data.eip != 0u ||
            fixture->instruction.cpu.data.ecx != 2u ||
            fixture->instruction.cpu.data.esi != 0x200u ||
            fixture->instruction.cpu.data.edi != 0x300u) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (fixture->port_transfers != 2u || fixture->instruction.fault.valid || fixture->bad_order)
            return LIB_FALSE;
    }
    for (lib_u8 opcode = 0x6cu; opcode <= 0x6eu; opcode += 2u) {
        x86_core_cpu_execution_context *context = &fixture->instruction.execution;

        smm_prepare(fixture);
        fixture->instruction.memory[0] = 0xf3u;
        fixture->instruction.memory[1] = opcode;
        fixture->io_smi = LIB_TRUE;
        x86_core_cpu_execution_refresh(context);
        if (fixture->instruction.cpu.data.eip != 2u || fixture->port_transfers ||
            context->io_restart_valid || fixture->active || !smm_enter(fixture) ||
            (smm_image_get(fixture, 0x7f00u) & 0xffffu)) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (context->smm_active || fixture->instruction.cpu.data.eip != 2u || fixture->port_transfers ||
            fixture->instruction.fault.valid) return LIB_FALSE;
    }
    {
        x86_core_cpu_execution_context *context = &fixture->instruction.execution;

        smm_prepare(fixture);
        fixture->instruction.memory[0] = 0xe6u;
        fixture->instruction.memory[1] = 0x80u;
        fixture->io_smi = LIB_TRUE;
        x86_core_cpu_execution_refresh(context);
        smm_image_put(fixture, 0x7f00u, 0xffu);
        if (x86_core_cpu_set_smi(context, LIB_FALSE) != LIB_STATUS_OK ||
            x86_core_cpu_set_smi(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
        fixture->io_smi = LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (!context->smm_active || fixture->assertions != 2u || fixture->port_transfers != 1u ||
            (smm_image_get(fixture, 0x7f00u) & 0xffffu)) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (context->smm_active || fixture->instruction.cpu.data.eip) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (fixture->port_transfers != 2u || fixture->instruction.fault.valid || fixture->bad_order)
            return LIB_FALSE;
    }
    smm_prepare(fixture);
    fixture->instruction.memory[0] = 0xe6u;
    fixture->instruction.memory[1] = 0x80u;
    fixture->fail_port = fixture->io_smi = LIB_TRUE;
    x86_core_cpu_execution_refresh(&fixture->instruction.execution);
    return fixture->instruction.fault.valid && fixture->port_transfers == 0u &&
        fixture->assertions == 0u && !fixture->instruction.execution.io_restart_valid;
}

static lib_bool smm_invalid_state(smm_fixture *fixture)
{
    static const struct {lib_u16 offset; lib_u32 value;} cases[] = {
        {0x7ef8u,0x30001u}, {0x7f28u,2u},
        {0x7ffcu,VCPU_CR0_ET | VCPU_CR0_PG}, {0x7ffcu,VCPU_CR0_ET | VCPU_CR0_NW}
    };

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    for (lib_u8 wake = 0u; wake < 3u; ++wake) {
        x86_core_cpu_execution_context *context = &fixture->instruction.execution;
        lib_u32 reads;

        smm_prepare(fixture);
        fixture->instruction.cpu.data.esp = 0x8000u;
        fixture->instruction.memory[0x09u] = fixture->instruction.memory[0xc1u] = 2u;
        if (!smm_enter(fixture) || x86_core_cpu_set_smi(context, LIB_FALSE) != LIB_STATUS_OK)
            return LIB_FALSE;
        smm_image_put(fixture, cases[index].offset, cases[index].value);
        x86_core_cpu_execution_refresh(context);
        if (!x86_core_cpu_is_shutdown(context) || context->shutdown_state != CPU_SHUTDOWN_SMM ||
            fixture->shutdown_cycles != 1u ||
            fixture->active || fixture->instruction.fault.valid ||
            !fixture->instruction.delivered_exception.valid ||
            fixture->instruction.delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN)
            return LIB_FALSE;
        reads = fixture->reads;
        x86_core_cpu_execution_refresh(context);
        if (fixture->reads != reads || !x86_core_cpu_is_shutdown(context) ||
            fixture->shutdown_cycles != 1u) return LIB_FALSE;
        if (wake == 0u) {
            if (!x86_core_cpu_request_nmi(context)) return LIB_FALSE;
        } else if (wake == 1u) fixture->interrupt = LIB_TRUE;
        else if (x86_core_cpu_set_smi(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (x86_core_cpu_is_shutdown(context) || fixture->instruction.fault.valid ||
            (wake < 2u && fixture->instruction.cpu.data.eip != 0x200u) ||
            (wake == 2u && !context->smm_active)) return LIB_FALSE;
        x86_core_cpu_execution_finalize(context);
    }
    return LIB_TRUE;
}

static lib_bool smm_preview_and_admission(smm_fixture *fixture)
{
    x86_core_cpu_execution_context before;
    x86_core_cpu_instruction_lexeme lexeme;
    x86_core_cpu_bus_provider bus = smm_bus;
    t_cpu cpu;
    x86_core_cpu_execution_context *context = &fixture->instruction.execution;

    smm_prepare(fixture);
    fixture->instruction.memory[0] = 0x0fu;
    fixture->instruction.memory[1] = 0xaau;
    fixture->instruction.memory[0x19u] = 2u;
    fixture->instruction.cpu.data.esp = 0x8000u;
    if (x86_core_cpu_set_smi(LIB_NULL, LIB_TRUE) != LIB_STATUS_INVALID_ARGUMENT) return LIB_FALSE;
    bus.system_management_active = LIB_NULL;
    context->bus = &bus;
    if (x86_core_cpu_set_smi(context, LIB_TRUE) != LIB_STATUS_UNSUPPORTED || context->smi_pending)
        return LIB_FALSE;
    context->bus = &smm_bus;
    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_DEFAULT;
         profile < X86_CORE_CPU_PROFILE_80486SX; ++profile) {
        context->cpu_profile = profile;
        if (x86_core_cpu_set_smi(context, LIB_TRUE) != LIB_STATUS_UNSUPPORTED ||
            context->smi_pending) return LIB_FALSE;
    }
    context->cpu_profile = X86_CORE_CPU_PROFILE_80486SX;
    before = *context;
    cpu = fixture->instruction.cpu;
    if (!x86_core_cpu_execution_preview_lexeme(context, &lexeme) ||
        !lexeme.available || lexeme.byte_count != 2u ||
        lib_memory_compare(&before, context, sizeof(before)) ||
        lib_memory_compare(&cpu, &fixture->instruction.cpu, sizeof(cpu)) || fixture->assertions)
        return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    if (fixture->assertions || fixture->instruction.fault.valid ||
        !fixture->instruction.delivered_exception.valid ||
        fixture->instruction.delivered_exception.exception_mask != VCPUINS_EXCEPT_UD ||
        fixture->instruction.cpu.data.eip != 0x200u) return LIB_FALSE;
    smm_prepare(fixture);
    if (x86_core_cpu_set_smi(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
    fixture->instruction.memory[0] = 0x0fu;
    fixture->instruction.memory[1] = 0xaau;
    before = *context;
    if (!x86_core_cpu_execution_preview_lexeme(context, &lexeme) ||
        lib_memory_compare(&before, context, sizeof(before)) || fixture->assertions)
        return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    if (!context->smm_active) return LIB_FALSE;
    before = *context;
    if (!x86_core_cpu_execution_preview_lexeme(context, &lexeme) ||
        lexeme.byte_count != 2u || lib_memory_compare(&before, context, sizeof(before)) ||
        fixture->reads || fixture->assertions != 1u) return LIB_FALSE;
    x86_core_cpu_execution_finalize(context);
    return !fixture->active && fixture->releases == 1u;
}

static lib_bool smm_rsm_form_failures(smm_fixture *fixture)
{
    x86_core_cpu_execution_context *context = &fixture->instruction.execution;
    x86_core_cpu_execution_context saved;
    x86_core_cpu_instruction_lexeme lexeme;
    t_cpu before;
    const lib_u8 locked[] = {0xf0u,0x0fu,0xaau};
    const lib_u8 vector[] = {0x20u,0x80u,0u,0x30u};

    smm_prepare(fixture);
    fixture->instruction.cpu.data.esp = 0x8000u;
    lib_memory_copy(fixture->instruction.memory + 0x18u, vector, sizeof(vector));
    lib_memory_copy(fixture->instruction.memory + 0x38000u, locked, sizeof(locked));
    if (!smm_enter(fixture)) return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    if (!context->smm_active || fixture->releases || fixture->reads || fixture->instruction.fault.valid ||
        !fixture->instruction.delivered_exception.valid ||
        fixture->instruction.delivered_exception.exception_mask != VCPUINS_EXCEPT_UD ||
        fixture->instruction.cpu.data.eip != 0x8020u) return LIB_FALSE;
    x86_core_cpu_execution_finalize(context);

    smm_prepare(fixture);
    if (!smm_enter(fixture)) return LIB_FALSE;
    fixture->instruction.cpu.data.cs.selector = 0x7ff0u;
    fixture->instruction.cpu.data.cs.base = 0x7ff00u;
    fixture->instruction.cpu.data.eip = 0xffu;
    fixture->instruction.memory[0x7ffffu] = 0x0fu;
    before = fixture->instruction.cpu;
    saved = *context;
    if (x86_core_cpu_execution_preview_lexeme(context, &lexeme) || lexeme.available ||
        lib_memory_compare(&saved, context, sizeof(saved)) ||
        lib_memory_compare(&before, &fixture->instruction.cpu, sizeof(before)) || fixture->reads ||
        fixture->releases) return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    return context->smm_active && fixture->instruction.fault.valid && !fixture->reads &&
        !fixture->releases && fixture->instruction.fault.exception_mask == VCPUINS_EXCEPT_CE &&
        fixture->instruction.fault.exception_code == 0x80000u &&
        !lib_memory_compare(&before, &fixture->instruction.cpu, sizeof(before)) && !fixture->bad_order;
}

static lib_bool smm_loadall_rejection(smm_fixture *fixture)
{
    static const lib_u8 opcodes[] = {0x05u, 0x07u};
    const lib_u8 vector[] = {0x20u, 0x80u, 0u, 0x30u};

    for (lib_size opcode = 0u; opcode < sizeof(opcodes); ++opcode)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 locked = 0u; locked < 2u; ++locked) {
        x86_core_cpu_execution_context *context = &fixture->instruction.execution;
        lib_u8 code[5], bytes = 0u;
        lib_u16 frame[3];

        smm_prepare(fixture);
        fixture->instruction.cpu.data.esp = 0x8000u;
        fixture->instruction.cpu.data.edi = 0x1000u;
        lib_memory_copy(fixture->instruction.memory + 0x18u, vector, sizeof(vector));
        if (locked) code[bytes++] = 0xf0u;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0x0fu;
        code[bytes++] = opcodes[opcode];
        lib_memory_copy(fixture->instruction.memory + 0x38000u, code, bytes);
        if (!smm_enter(fixture)) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (!context->smm_active || !fixture->active || fixture->releases ||
            fixture->reads || fixture->instruction.fault.valid ||
            !fixture->instruction.delivered_exception.valid ||
            fixture->instruction.delivered_exception.exception_mask != VCPUINS_EXCEPT_UD ||
            fixture->instruction.delivered_exception.exception_code != 0u ||
            fixture->instruction.delivered_exception.point.cs != 0x3000u ||
            fixture->instruction.delivered_exception.point.eip != 0x8000u ||
            fixture->instruction.cpu.data.eip != 0x8020u ||
            fixture->instruction.cpu.data.esp != 0x7ffau ||
            fixture->instruction.cpu.data.edi != 0x1000u || fixture->bad_order)
            return LIB_FALSE;
        lib_memory_copy(frame, fixture->instruction.memory + 0x7ffau, sizeof(frame));
        if (frame[0] != 0x8000u || frame[1] != 0x3000u) return LIB_FALSE;
        x86_core_cpu_execution_finalize(context);
        if (fixture->active || fixture->releases != 1u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool smm_entry_priority(smm_fixture *fixture)
{
    for (lib_u8 kind = 0u; kind < 3u; ++kind) {
        x86_core_cpu_execution_context *context = &fixture->instruction.execution;
        const lib_u8 vector = kind == 0u ? 1u : kind == 1u ? 2u : 0x30u;

        smm_prepare(fixture);
        fixture->instruction.cpu.data.esp = 0x8000u;
        fixture->instruction.cpu.data.eflags |= VCPU_EFLAGS_IF;
        fixture->instruction.memory[vector * 4u + 1u] = 2u;
        if (kind == 0u) {
            context->debug_trap_pending = LIB_TRUE;
            context->debug_trap_cause = 0x00004000u;
        } else if (kind == 1u) {
            if (!x86_core_cpu_request_nmi(context)) return LIB_FALSE;
        } else fixture->interrupt = LIB_TRUE;
        if (!smm_enter(fixture) || fixture->instruction.cpu.data.eip != 0x8000u ||
            fixture->instruction.delivered_exception.valid ||
            (kind == 1u && !context->nmi_pending) ||
            (kind == 2u && !fixture->interrupt)) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (context->smm_active || fixture->instruction.cpu.data.eip != 0x200u ||
            fixture->instruction.fault.valid || context->debug_trap_pending ||
            (kind == 0u && (!fixture->instruction.delivered_exception.valid ||
                fixture->instruction.delivered_exception.exception_mask != VCPUINS_EXCEPT_DB)) ||
            (kind == 1u && (context->nmi_pending || !context->nmi_in_service)) ||
            (kind == 2u && fixture->interrupt)) return LIB_FALSE;
    }
    smm_prepare(fixture);
    fixture->instruction.cpu.data.cs.base = 0x90000u;
    if (!smm_enter(fixture)) return LIB_FALSE;
    x86_core_cpu_execution_refresh(&fixture->instruction.execution);
    return !fixture->instruction.execution.smm_active && !fixture->instruction.fault.valid &&
        fixture->instruction.cpu.data.cs.base == 0x90000u;
}

static lib_bool smm_nmi_and_cache(smm_fixture *fixture)
{
    const lib_u8 nmi_vector[] = {0x20u,0x80u,0u,0x30u};
    const lib_u8 dummy_vector[] = {0x10u,0x80u,0u,0x30u};
    const lib_u8 handler[] = {0xcdu,0x40u,0x0fu,0xaau};
    const lib_u32 positions[] = {0x8010u,0x8020u,0x8002u,0u};
    x86_core_cpu_execution_context *context = &fixture->instruction.execution;
    cpu_cache_line cached = {.tag = 0x70000u, .data = {0x11223344u,0u,0u,0u}, .valid = LIB_TRUE};

    smm_prepare(fixture);
    fixture->instruction.cpu.data.esp = 0x8000u;
    context->cache[0][0] = cached;
    context->tlb[0][0] = (cpu_tlb_entry){.linear = 0x1000u,
        .physical = 0x2000u, .valid = LIB_TRUE};
    lib_memory_copy(fixture->instruction.memory + 8u, nmi_vector, sizeof(nmi_vector));
    lib_memory_copy(fixture->instruction.memory + 0x100u, dummy_vector, sizeof(dummy_vector));
    lib_memory_copy(fixture->instruction.memory + 0x38000u, handler, sizeof(handler));
    fixture->instruction.memory[0x38010u] = fixture->instruction.memory[0x38020u] = 0xcfu;
    if (!smm_enter(fixture) || context->tlb[0][0].valid ||
        lib_memory_compare(&cached, &context->cache[0][0], sizeof(cached)) ||
        !x86_core_cpu_request_nmi(context)) return LIB_FALSE;
    for (lib_u8 step = 0u; step < 4u; ++step) {
        x86_core_cpu_execution_refresh(context);
        if (fixture->instruction.cpu.data.eip != positions[step] ||
            context->smm_active != (step < 3u) || fixture->instruction.fault.valid ||
            lib_memory_compare(&cached, &context->cache[0][0], sizeof(cached))) return LIB_FALSE;
        if (step == 0u && (!context->nmi_pending || !context->nmi_in_service)) return LIB_FALSE;
        if (step == 1u && (context->nmi_pending || !context->nmi_in_service)) return LIB_FALSE;
    }
    return fixture->releases == 1u && !fixture->bad_order;
}

static lib_bool smm_handler_intr(smm_fixture *fixture)
{
    const lib_u8 vector[] = {0x20u,0x80u,0u,0x30u};
    const lib_u8 handler[] = {0xfbu,0x90u,0x0fu,0xaau};
    const lib_u32 positions[] = {0x8001u,0x8020u,0x8002u,0u};
    x86_core_cpu_execution_context *context = &fixture->instruction.execution;

    smm_prepare(fixture);
    fixture->instruction.cpu.data.esp = 0x8000u;
    fixture->interrupt = LIB_TRUE;
    lib_memory_copy(fixture->instruction.memory + 0xc0u, vector, sizeof(vector));
    lib_memory_copy(fixture->instruction.memory + 0x38000u, handler, sizeof(handler));
    fixture->instruction.memory[0x38020u] = 0xcfu;
    if (!smm_enter(fixture) || !fixture->interrupt ||
        (fixture->instruction.cpu.data.eflags & VCPU_EFLAGS_IF)) return LIB_FALSE;
    for (lib_u8 step = 0u; step < 4u; ++step) {
        x86_core_cpu_execution_refresh(context);
        if (fixture->instruction.cpu.data.eip != positions[step] ||
            context->smm_active != (step < 3u) || fixture->instruction.fault.valid ||
            fixture->interrupt != (step == 0u)) return LIB_FALSE;
        if (step == 0u && (!(fixture->instruction.cpu.data.eflags & VCPU_EFLAGS_IF) ||
            !context->nmi_in_service)) return LIB_FALSE;
        if (step == 1u && (fixture->instruction.cpu.data.eflags & VCPU_EFLAGS_IF))
            return LIB_FALSE;
    }
    return !(fixture->instruction.cpu.data.eflags & VCPU_EFLAGS_IF) &&
        fixture->instruction.cpu.data.esp == 0x8000u &&
        fixture->releases == 1u && !fixture->bad_order;
}

static lib_bool smm_handler_debug(smm_fixture *fixture)
{
    const lib_u8 vector[] = {0x20u,0x80u,0u,0x30u};
    const lib_u8 handler[] = {0x68u,2u,1u,0x9du,0x90u,0x0fu,0xaau};
    const lib_u32 positions[] = {0x8003u,0x8004u,0x8020u,0x8005u,0u};
    x86_core_cpu_execution_context *context = &fixture->instruction.execution;

    smm_prepare(fixture);
    fixture->instruction.cpu.data.esp = 0x8000u;
    lib_memory_copy(fixture->instruction.memory + 4u, vector, sizeof(vector));
    lib_memory_copy(fixture->instruction.memory + 0x38000u, handler, sizeof(handler));
    fixture->instruction.memory[0x38020u] = 0xcfu;
    if (!smm_enter(fixture)) return LIB_FALSE;
    for (lib_size step = 0u; step < sizeof(positions) / sizeof(positions[0]); ++step) {
        x86_core_cpu_execution_refresh(context);
        if (fixture->instruction.cpu.data.eip != positions[step] ||
            context->smm_active != (step < 4u) || fixture->instruction.fault.valid)
            return LIB_FALSE;
    }
    return fixture->instruction.delivered_exception.valid &&
        fixture->instruction.delivered_exception.exception_mask == VCPUINS_EXCEPT_DB &&
        !(fixture->instruction.cpu.data.eflags & VCPU_EFLAGS_TF) && !context->debug_trap_pending &&
        fixture->instruction.cpu.data.esp == 0x8000u && fixture->releases == 1u && !fixture->bad_order;
}

static lib_bool smm_lock_boundary(smm_fixture *fixture)
{
    static const struct {lib_u8 code[6]; lib_u8 bytes;} cases[] = {
        {{0xf0u,0x0fu,0xc1u,0x06u,0u,2u},6u},
        {{0x87u,0x06u,0u,2u,0u,0u},4u}
    };

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        x86_core_cpu_execution_context *context = &fixture->instruction.execution;
        lib_u16 value;

        smm_prepare(fixture);
        lib_memory_copy(fixture->instruction.memory, cases[index].code, cases[index].bytes);
        fixture->instruction.cpu.data.ax = 2u;
        fixture->instruction.memory[0x200u] = 3u;
        fixture->memory_smi = LIB_TRUE;
        x86_core_cpu_execution_refresh(context);
        lib_memory_copy(&value, fixture->instruction.memory + 0x200u, sizeof(value));
        if (!context->smm_active || fixture->locked || fixture->lock_begins != 1u ||
            fixture->lock_commits != 1u || fixture->bad_order ||
            fixture->instruction.fault.valid || smm_image_get(fixture, 0x7ff0u) != cases[index].bytes ||
            smm_image_get(fixture, 0x7fd0u) != 3u || value != (index == 0u ? 5u : 2u))
            return LIB_FALSE;
        x86_core_cpu_execution_finalize(context);
        smm_prepare(fixture);
        lib_memory_copy(fixture->instruction.memory, cases[index].code, cases[index].bytes);
        fixture->instruction.cpu.data.ax = 2u;
        fixture->instruction.memory[0x200u] = 3u;
        fixture->memory_stop_clock = LIB_TRUE;
        x86_core_cpu_execution_refresh(context);
        lib_memory_copy(&value, fixture->instruction.memory + 0x200u, sizeof(value));
        if (fixture->locked || fixture->lock_begins != 1u || fixture->lock_commits != 1u ||
            fixture->cycle_polls || fixture->bad_order || fixture->instruction.fault.valid ||
            fixture->instruction.cpu.data.eip != cases[index].bytes ||
            fixture->instruction.cpu.data.ax != 3u || value != (index == 0u ? 5u : 2u))
            return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (!context->stop_granted || fixture->stop_cycles != 1u || fixture->bad_order)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool smm_power_wait(smm_fixture *fixture)
{
    x86_core_cpu_execution_context *context = &fixture->instruction.execution;
    x86_core_cpu_execution_context saved;
    x86_core_cpu_instruction_lexeme lexeme;
    lib_u32 polls;

    smm_prepare(fixture);
    fixture->instruction.memory[0] = 0x90u;
    fixture->cycle_ready = LIB_FALSE;
    if (x86_core_cpu_set_clock_running(context, LIB_FALSE) != LIB_STATUS_INVALID_STATE ||
        x86_core_cpu_set_stop_clock(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    if (!x86_core_cpu_is_system_waiting(context) || context->stop_granted ||
        fixture->held_cycle != X86_CORE_CPU_SPECIAL_STOP_GRANT || fixture->stop_cycles ||
        fixture->instruction.cpu.data.eip || fixture->instruction.fault.valid ||
        x86_core_cpu_capture_timing(context).ticks) return LIB_FALSE;
    saved = *context;
    if (!x86_core_cpu_execution_preview_lexeme(context, &lexeme) ||
        lib_memory_compare(&saved, context, sizeof(saved)) || fixture->cycle_polls != 1u)
        return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    if (fixture->cycle_polls != 2u || context->stop_granted) return LIB_FALSE;
    fixture->cycle_ready = LIB_TRUE;
    x86_core_cpu_execution_refresh(context);
    polls = fixture->cycle_polls;
    if (!context->stop_granted || fixture->stop_cycles != 1u ||
        x86_core_cpu_set_clock_running(context, LIB_FALSE) != LIB_STATUS_OK ||
        x86_core_cpu_set_stop_clock(context, LIB_FALSE) != LIB_STATUS_INVALID_STATE ||
        x86_core_cpu_set_flush(context, LIB_TRUE) != LIB_STATUS_INVALID_STATE ||
        x86_core_cpu_set_nmi(context, LIB_TRUE) != LIB_STATUS_OK ||
        x86_core_cpu_set_nmi(context, LIB_FALSE) != LIB_STATUS_OK ||
        x86_core_cpu_set_smi(context, LIB_TRUE) != LIB_STATUS_OK ||
        x86_core_cpu_set_smi(context, LIB_FALSE) != LIB_STATUS_OK) return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    if (fixture->cycle_polls != polls || context->nmi_pending || context->smi_pending ||
        x86_core_cpu_set_clock_running(context, LIB_TRUE) != LIB_STATUS_OK ||
        x86_core_cpu_set_flush(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
    fixture->instruction.cpu.data.cr0 &= ~VCPU_CR0_NW;
    context->cache[0][0] = (cpu_cache_line){.tag = 0x70000u, .valid = LIB_TRUE};
    x86_core_cpu_signal_cache_write(context, 0x70000u, 1u);
    if (context->cache[0][0].valid || !context->stop_granted || fixture->cycle_polls != polls)
        return LIB_FALSE;
    context->cache[0][0].valid = LIB_TRUE;
    x86_core_cpu_execution_refresh(context);
    if (!context->cache[0][0].valid || !context->flush_pending ||
        fixture->cycle_polls != polls ||
        x86_core_cpu_set_flush(context, LIB_FALSE) != LIB_STATUS_OK ||
        x86_core_cpu_set_stop_clock(context, LIB_FALSE) != LIB_STATUS_OK) return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    return !x86_core_cpu_is_system_waiting(context) && !context->cache[0][0].valid &&
        !context->flush_pending && fixture->instruction.cpu.data.eip == 1u &&
        !fixture->instruction.fault.valid && fixture->cycle_polls == polls && !fixture->bad_order;
}

static lib_bool smm_power_wake(smm_fixture *fixture)
{
    x86_core_cpu_execution_context *context = &fixture->instruction.execution;

    for (lib_u8 input = 0u; input < 3u; ++input) {
        smm_prepare(fixture);
        fixture->instruction.cpu.data.esp = 0x8000u;
        fixture->instruction.cpu.data.eflags |= VCPU_EFLAGS_IF;
        fixture->instruction.memory[9u] = fixture->instruction.memory[0xc1u] = 2u;
        if (x86_core_cpu_set_stop_clock(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (!context->stop_granted) return LIB_FALSE;
        if (input == 0u) {
            if (x86_core_cpu_set_nmi(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
        } else if (input == 1u) {
            if (x86_core_cpu_set_smi(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
        } else fixture->interrupt = LIB_TRUE;
        x86_core_cpu_execution_refresh(context);
        if (context->smi_pending || context->nmi_pending || fixture->active ||
            fixture->instruction.cpu.data.eip ||
            x86_core_cpu_set_stop_clock(context, LIB_FALSE) != LIB_STATUS_OK) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (context->stop_granted || fixture->instruction.fault.valid ||
            fixture->instruction.cpu.data.eip != (input == 1u ? 0x8000u : 0x200u) ||
            fixture->active != (input == 1u) ||
            x86_core_cpu_capture_timing(context).ticks || fixture->bad_order) return LIB_FALSE;
        x86_core_cpu_execution_finalize(context);
    }
    smm_prepare(fixture);
    fixture->instruction.memory[0] = 0xf4u;
    fixture->instruction.cpu.data.esp = 0x8000u;
    fixture->instruction.memory[9u] = 2u;
    x86_core_cpu_execution_refresh(context);
    if (fixture->halt_cycles != 1u || !x86_core_cpu_is_halted(context) ||
        x86_core_cpu_set_stop_clock(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    if (!context->stop_granted ||
        x86_core_cpu_set_flush(context, LIB_TRUE) != LIB_STATUS_OK ||
        x86_core_cpu_set_flush(context, LIB_FALSE) != LIB_STATUS_OK ||
        x86_core_cpu_set_stop_clock(context, LIB_FALSE) != LIB_STATUS_OK) return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    context->cache[0][0].valid = LIB_TRUE;
    x86_core_cpu_execution_refresh(context);
    if (!x86_core_cpu_is_halted(context) || fixture->halt_cycles != 2u ||
        fixture->stop_cycles != 1u || !context->flush_pending || !context->cache[0][0].valid ||
        x86_core_cpu_set_nmi(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    return !context->flush_pending && !context->cache[0][0].valid &&
        !x86_core_cpu_is_halted(context) && fixture->instruction.cpu.data.eip == 0x200u &&
        fixture->halt_cycles == 2u && !fixture->instruction.fault.valid && !fixture->bad_order;
}

static lib_bool smm_flush_and_priority(smm_fixture *fixture)
{
    x86_core_cpu_execution_context *context = &fixture->instruction.execution;

    smm_prepare(fixture);
    fixture->instruction.memory[0] = 0x90u;
    if (x86_core_cpu_set_flush(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
    for (lib_u8 round = 0u; round < 2u; ++round) {
        context->cache[0][0].valid = LIB_TRUE;
        x86_core_cpu_execution_refresh(context);
        if (context->cache[0][0].valid || fixture->instruction.cpu.data.eip ||
            x86_core_cpu_capture_timing(context).ticks || !context->instruction_wait_stall ||
            fixture->cycle_polls) return LIB_FALSE;
    }
    if (x86_core_cpu_set_flush(context, LIB_FALSE) != LIB_STATUS_OK) return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    if (fixture->instruction.cpu.data.eip != 1u || fixture->instruction.fault.valid) return LIB_FALSE;

    smm_prepare(fixture);
    context->cache[0][0].valid = LIB_TRUE;
    if (x86_core_cpu_set_stop_clock(context, LIB_TRUE) != LIB_STATUS_OK ||
        x86_core_cpu_set_flush(context, LIB_TRUE) != LIB_STATUS_OK ||
        x86_core_cpu_set_smi(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    if (context->cache[0][0].valid || context->smm_active || fixture->cycle_polls ||
        !context->smi_pending ||
        x86_core_cpu_set_flush(context, LIB_FALSE) != LIB_STATUS_OK) return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    if (!context->smm_active || fixture->stop_cycles || fixture->instruction.fault.valid)
        return LIB_FALSE;
    x86_core_cpu_execution_finalize(context);
    smm_prepare(fixture);
    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_DEFAULT;
        profile < X86_CORE_CPU_PROFILE_80486SX; ++profile) {
        context->cpu_profile = profile;
        if (x86_core_cpu_set_stop_clock(context, LIB_TRUE) != LIB_STATUS_UNSUPPORTED ||
            x86_core_cpu_set_clock_running(context, LIB_FALSE) != LIB_STATUS_UNSUPPORTED ||
            x86_core_cpu_set_flush(context, LIB_TRUE) != LIB_STATUS_UNSUPPORTED ||
            x86_core_cpu_set_nmi(context, LIB_TRUE) != LIB_STATUS_UNSUPPORTED ||
            context->stop_clock_asserted || context->flush_pending || context->nmi_pending)
            return LIB_FALSE;
    }
    context->cpu_profile = X86_CORE_CPU_PROFILE_80486SX;
    {
        x86_core_cpu_bus_provider missing = smm_bus;

        missing.special_cycle = LIB_NULL;
        context->bus = &missing;
        if (x86_core_cpu_set_stop_clock(context, LIB_TRUE) != LIB_STATUS_UNSUPPORTED)
            return LIB_FALSE;
        fixture->instruction.memory[0] = 0xf4u;
        x86_core_cpu_execution_refresh(context);
        context->bus = &smm_bus;
    }
    return fixture->instruction.fault.valid &&
        fixture->instruction.fault.exception_mask == VCPUINS_EXCEPT_CE &&
        fixture->instruction.fault.exception_code == LIB_STATUS_UNSUPPORTED && !fixture->bad_order;
}

static lib_bool smm_power_rep(smm_fixture *fixture)
{
    x86_core_cpu_execution_context *context = &fixture->instruction.execution;

    for (lib_u8 opcode = 0x6cu; opcode <= 0x6eu; opcode += 2u) {
        smm_prepare(fixture);
        fixture->instruction.memory[0] = 0xf3u;
        fixture->instruction.memory[1] = opcode;
        fixture->instruction.cpu.data.ecx = 2u;
        fixture->instruction.cpu.data.esi = 0x200u;
        fixture->instruction.cpu.data.edi = 0x300u;
        fixture->instruction.cpu.data.dx = 0x80u;
        fixture->io_stop_clock = LIB_TRUE;
        x86_core_cpu_execution_refresh(context);
        if (fixture->instruction.cpu.data.ecx != 1u || fixture->instruction.cpu.data.eip ||
            fixture->port_transfers != 1u || fixture->cycle_polls || fixture->instruction.fault.valid)
            return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (!context->stop_granted || fixture->port_transfers != 1u || fixture->stop_cycles != 1u)
            return LIB_FALSE;
        fixture->io_stop_clock = LIB_FALSE;
        if (x86_core_cpu_set_stop_clock(context, LIB_FALSE) != LIB_STATUS_OK) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (context->stop_granted || fixture->instruction.cpu.data.ecx ||
            fixture->instruction.cpu.data.eip != 2u || fixture->port_transfers != 2u ||
            fixture->instruction.cpu.data.esi != (opcode == 0x6eu ? 0x202u : 0x200u) ||
            fixture->instruction.cpu.data.edi != (opcode == 0x6cu ? 0x302u : 0x300u) ||
            fixture->instruction.fault.valid || fixture->bad_order) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool smm_power_cancel(smm_fixture *fixture)
{
    x86_core_cpu_execution_context *context = &fixture->instruction.execution;

    for (lib_u8 cause = 0u; cause < 3u; ++cause) {
        smm_prepare(fixture);
        context->smbase = 0x50000u;
        fixture->cycle_ready = LIB_FALSE;
        if (x86_core_cpu_set_stop_clock(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (cause == 0u) x86_core_cpu_state_reset(context);
        else if (cause == 1u) {
            if (x86_core_cpu_state_soft_reset(context) != LIB_STATUS_OK) return LIB_FALSE;
        } else x86_core_cpu_execution_finalize(context);
        if (fixture->cycle_cancels != 1u || fixture->held_cycle != X86_CORE_CPU_SPECIAL_NONE ||
            x86_core_cpu_is_system_waiting(context) || fixture->bad_order ||
            (cause < 2u && (!context->clock_running || context->stop_clock_asserted ||
                context->smbase != (cause == 1u ? 0x50000u : 0x30000u)))) return LIB_FALSE;
    }
    for (lib_u8 soft = 0u; soft < 2u; ++soft) {
        smm_prepare(fixture);
        context->smbase = 0x50000u;
        fixture->image_base = fixture->next_image_base = 0x5fe00u;
        if (!smm_enter(fixture)) return LIB_FALSE;
        if (soft) {
            if (x86_core_cpu_state_soft_reset(context) != LIB_STATUS_OK) return LIB_FALSE;
        } else x86_core_cpu_state_reset(context);
        if (fixture->active || fixture->releases != 1u || context->smm_active || context->smi_pending ||
            context->smbase != (soft ? 0x50000u : 0x30000u) || fixture->bad_order) return LIB_FALSE;
    }
    smm_prepare(fixture);
    x86_core_cpu_execution_request_shutdown(context);
    x86_core_cpu_state_reset(context);
    if (fixture->cycle_polls || fixture->cycle_cancels || fixture->bad_order ||
        x86_core_cpu_is_system_waiting(context)) return LIB_FALSE;
    x86_core_cpu_set_nmi_mask(context, LIB_TRUE);
    if (x86_core_cpu_set_nmi(context, LIB_TRUE) != LIB_STATUS_OK || context->nmi_pending)
        return LIB_FALSE;
    x86_core_cpu_set_nmi_mask(context, LIB_FALSE);
    if (!context->nmi_pending) return LIB_FALSE;
    smm_prepare(fixture);
    fixture->cycle_fail = LIB_TRUE;
    if (x86_core_cpu_set_stop_clock(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
    x86_core_cpu_execution_refresh(context);
    return fixture->instruction.fault.valid && fixture->instruction.execution.stop_requested &&
        fixture->instruction.fault.exception_mask == VCPUINS_EXCEPT_CE &&
        fixture->instruction.fault.exception_code == LIB_STATUS_IO_ERROR &&
        !context->stop_granted && fixture->cycle_cancels == 1u && !fixture->bad_order;
}

static lib_bool smm_suspend_image(smm_fixture *fixture)
{
    lib_u8 saved[512];
    t_cpu expected;
    x86_core_cpu_execution_context *context = &fixture->instruction.execution;

    smm_prepare(fixture);
    fixture->instruction.cpu.data.eax = 0x12345678u;
    fixture->instruction.cpu.data.eip = 0x123u;
    fixture->instruction.cpu.data.cr0 |= VCPU_CR0_PE;
    fixture->instruction.cpu.data.cr1 = 0x98765432u;
    fixture->instruction.cpu.data.cr2 = 0x76543210u;
    fixture->instruction.cpu.data.cr3 = 0x70000u;
    fixture->instruction.cpu.data.cr4 = VCPU_CR4_VME;
    fixture->instruction.cpu.data.cs.selector = 0x28u;
    fixture->instruction.cpu.data.cs.base = 0x4000u;
    fixture->instruction.cpu.data.cs.limit = 0x000fffffu;
    fixture->instruction.cpu.data.cs.seg.exec.defsize = LIB_TRUE;
    fixture->instruction.cpu.data.ss.limit = 0x001fffffu;
    fixture->instruction.cpu.data.ss.seg.data.big = LIB_TRUE;
    fixture->instruction.cpu.data.ldtr.base = 0x6000u;
    fixture->instruction.cpu.data.ldtr.limit = 0x3456u;
    fixture->instruction.cpu.data.dr0 = 0xaabbccddu;
    fixture->instruction.cpu.data.tr3 = 0xabcdef01u;
    expected = fixture->instruction.cpu;
    expected.data.dr0 = expected.data.tr3 = 0u;
    if (!smm_enter(fixture)) return LIB_FALSE;
    lib_memory_copy(saved, fixture->instruction.memory + fixture->image_base, sizeof(saved));
    /* Simulated nonvolatile image, not a production host checkpoint: RESET
     * and a new SMI overwrite all live CPU/decoder and physical saved state. */
    x86_core_cpu_state_reset(context);
    if (!smm_enter(fixture)) return LIB_FALSE;
    lib_memory_copy(fixture->instruction.memory + fixture->image_base, saved, sizeof(saved));
    x86_core_cpu_execution_refresh(context);
    return !context->smm_active && !fixture->instruction.fault.valid &&
        !lib_memory_compare(&expected, &fixture->instruction.cpu, sizeof(expected)) &&
        fixture->assertions == 2u && fixture->releases == 2u && !fixture->bad_order;
}

static lib_bool smm_compare_checkpoint(smm_fixture *fixture)
{
    for (lib_u8 scenario = 0u; scenario < 4u; ++scenario) {
        const lib_bool repne = (scenario & 1u) != 0u;
        const lib_u8 code[] = {repne ? 0xf2u : 0xf3u,
            (scenario & 2u) != 0u ? 0xaeu : 0xa6u};
        x86_core_cpu_execution_context *context = &fixture->instruction.execution;
        t_cpu *cpu = &fixture->instruction.cpu;
        lib_u32 last_flags;

        smm_prepare(fixture);
        cpu->data.eax = 0x10u;
        cpu->data.esi = 0x500u;
        cpu->data.edi = 0x600u;
        cpu->data.ecx = 3u;
        cpu->data.eflags = 2u | VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_OF;
        lib_memory_set(fixture->instruction.memory + 0x500u, 0x10u, 3u);
        lib_memory_set(fixture->instruction.memory + 0x600u, repne ? 0x0fu : 0x10u, 3u);
        lib_memory_copy(fixture->instruction.memory, code, sizeof(code));
        x86_core_cpu_execution_refresh(context);
        last_flags = cpu->data.eflags;
        if (!context->repeat_compare_flags_valid || cpu->data.ecx != 2u ||
            cpu->data.eip != 0u || fixture->instruction.fault.valid) return LIB_FALSE;
        if (!smm_enter(fixture) || context->repeat_compare_flags_valid ||
            smm_image_get(fixture, 0x7ff4u) != last_flags) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (context->smm_active || context->repeat_compare_flags_valid ||
            cpu->data.eip != 0u || cpu->data.ecx != 2u || cpu->data.eflags != last_flags)
            return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (cpu->data.ecx != 1u || !context->repeat_compare_flags_valid ||
            context->repeat_compare_flags != last_flags) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (cpu->data.ecx != 0u || cpu->data.eip != sizeof(code) ||
            context->repeat_compare_flags_valid || fixture->instruction.fault.valid) return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    smm_fixture *fixture = lib_allocate_zero(1u, sizeof(*fixture));
    static const struct {
        const char *name;
        lib_bool (*run)(smm_fixture *);
    } cases[] = {
        {"round-trip",smm_round_trip}, {"guest-changes",smm_guest_changes},
        {"relocation-pending",smm_relocation_and_pending}, {"failures",smm_failures},
        {"halt-restart",smm_halt_restart}, {"io-restart",smm_io_restart},
        {"invalid-state",smm_invalid_state}, {"preview-admission",smm_preview_and_admission},
        {"loadall-rejection",smm_loadall_rejection},
        {"rsm-form-failures",smm_rsm_form_failures},
        {"entry-priority",smm_entry_priority}, {"nmi-cache",smm_nmi_and_cache},
        {"handler-intr",smm_handler_intr}, {"handler-debug",smm_handler_debug},
        {"lock-boundary",smm_lock_boundary}, {"suspend-image",smm_suspend_image},
        {"power-wait",smm_power_wait}, {"power-wake",smm_power_wake},
        {"power-cancel",smm_power_cancel}, {"flush-priority",smm_flush_and_priority},
        {"power-rep",smm_power_rep}, {"compare-checkpoint",smm_compare_checkpoint}
    };
    lib_bool passed = LIB_TRUE;

    if (fixture == LIB_NULL) return 1;
    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        if (!cases[index].run(fixture)) {
            lib_c_fprintf(lib_c_stderr, "SMM case failed: %s\n", cases[index].name);
            passed = LIB_FALSE;
            break;
        }
    }
    x86_core_cpu_execution_finalize(&fixture->instruction.execution);
    lib_release(fixture);
    if (!passed) return 1;
    lib_c_printf("CPU-SMM:OK\n");
    return 0;
}
