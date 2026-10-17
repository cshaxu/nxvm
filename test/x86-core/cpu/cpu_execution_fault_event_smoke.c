#include "support/cpu_bus_fixture.h"
#include "support/cpu_instruction_fixture.h"
#include "x86-core/cpu/cpu_timing.h"
#include "lib/types/file.h"

/* Private pending-event and whole-cache invariants belong to the CPU owner;
 * the board interrupt corpus separately checks PIC, frames and transactions. */
static void cpu_interrupt_prepare(cpu_bus_fixture *fixture,
    x86_core_cpu_profile profile, lib_u8 vector, lib_u8 gate_type)
{
    const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0xfau,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0
    };
    const lib_u8 gate[] = {0u,1u,0x0bu,0u,0u,gate_type,0u,0u};
    t_cpu *cpu = &fixture->cpu;

    cpu_bus_prepare(fixture, profile);
    lib_memory_copy(fixture->memory + 0x300u, gdt, sizeof(gdt));
    lib_memory_copy(fixture->memory + 0x400u + vector * 8u, gate, sizeof(gate));
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = LIB_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = 0x300u;
    cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.idtr.flagValid = LIB_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = 0x400u;
    cpu->data.idtr.limit = 0x187u;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.selector = 0x0bu;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = 0x2000u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = 3u;
    cpu->data.cs.seg.accessed = LIB_FALSE;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.cs.seg.exec.defsize = LIB_TRUE;
    cpu->data.cs.seg.exec.conform = LIB_FALSE;
    cpu->data.cs.seg.exec.readable = LIB_TRUE;
    cpu->data.ss.flagValid = LIB_TRUE;
    cpu->data.ss.selector = 0x10u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = 0xffffffffu;
    cpu->data.ss.dpl = 0u;
    cpu->data.ss.seg.accessed = LIB_FALSE;
    cpu->data.ss.seg.executable = LIB_FALSE;
    cpu->data.ss.seg.data.big = LIB_TRUE;
    cpu->data.ss.seg.data.expdown = LIB_FALSE;
    cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.eip = 0u;
    cpu->data.esp = 0x8000u;
    cpu->data.eflags = 0x302u;
    cpu->data.flagHalt = LIB_FALSE;
}

/* Preserve the full private-cache comparisons from the board UD corpus;
 * a copied public snapshot deliberately does not expose cache bookkeeping. */
static lib_i32 cpu_ud_cache_preservation(void)
{
    static const lib_u8 forms[][3] = {
        {0xf1u, 0u, 0u}, {0x0fu, 0x01u, 0xf8u},
        {0x0fu, 0x25u, 0xc0u}, {0x62u, 0xc0u, 0u},
        {0xf0u, 0x90u, 0u}
    };

    for (lib_size index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
        for (lib_u8 reject = 0u; reject != 2u; ++reject) {
            cpu_bus_fixture fixture;
            t_cpu before;

            cpu_interrupt_prepare(&fixture, X86_CORE_CPU_PROFILE_80386,
                6u, reject ? 0x80u : 0x8eu);
            /* The original UD cases use ring-zero 16-bit protected code. */
            fixture.memory[0x30du] = 0x9au;
            fixture.memory[0x30eu] = 0u;
            fixture.memory[0x432u] = 0x08u;
            fixture.cpu.data.cs.selector = 0x08u;
            fixture.cpu.data.cs.dpl = 0u;
            fixture.cpu.data.cs.seg.exec.defsize = LIB_FALSE;
            fixture.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF |
                VCPU_EFLAGS_DF;
            lib_memory_copy(fixture.memory, forms[index], sizeof(forms[index]));
            before = fixture.cpu;
            x86_core_cpu_execution_refresh(&fixture.execution);
            if (lib_memory_compare(&fixture.cpu.data.es, &before.data.es,
                    sizeof(before.data.es)) ||
                lib_memory_compare(&fixture.cpu.data.ss, &before.data.ss,
                    sizeof(before.data.ss)) ||
                lib_memory_compare(&fixture.cpu.data.ds, &before.data.ds,
                    sizeof(before.data.ds)) ||
                lib_memory_compare(&fixture.cpu.data.fs, &before.data.fs,
                    sizeof(before.data.fs)) ||
                lib_memory_compare(&fixture.cpu.data.gs, &before.data.gs,
                    sizeof(before.data.gs))) return 1;
            if (reject) {
                /* UD entry fails with GP; its GP entry fails with another GP,
                 * requiring DF. The absent DF handler cannot service it. */
                if (!x86_core_cpu_is_shutdown(&fixture.execution) || fixture.faults ||
                    fixture.execution.stop_requested ||
                    fixture.cpu.data.eip != before.data.eip ||
                    fixture.cpu.data.esp != before.data.esp ||
                    fixture.cpu.data.eflags != before.data.eflags) return 1;
            } else if (fixture.faults || fixture.cpu.data.eip != 0x100u ||
                fixture.cpu.data.esp != before.data.esp - 12u) return 1;
        }
    }
    return 0;
}

static lib_i32 cpu_interrupt_pending_and_rollback(void)
{
    for (lib_u8 reject = 0u; reject != 2u; ++reject) {
        cpu_bus_fixture fixture;
        t_cpu before;

        cpu_interrupt_prepare(&fixture, X86_CORE_CPU_PROFILE_80386,
            2u, reject ? 0x80u : 0x8eu);
        fixture.memory[0] = 0x90u;
        fixture.cpu.data.eflags = 0x202u;
        if (!x86_core_cpu_request_nmi(&fixture.execution)) return 1;
        before = fixture.cpu;
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.nmi_pending != reject) return 1;
        if (reject) {
            if (!x86_core_cpu_is_shutdown(&fixture.execution) || fixture.faults ||
                fixture.cpu.data.esp != before.data.esp ||
                fixture.cpu.data.eflags != before.data.eflags ||
                lib_memory_compare(&fixture.cpu.data.cs, &before.data.cs,
                    sizeof(before.data.cs)) ||
                lib_memory_compare(&fixture.cpu.data.ss, &before.data.ss,
                    sizeof(before.data.ss))) return 1;
        } else if (fixture.faults || fixture.cpu.data.esp != 0x7ff4u ||
            fixture.cpu.data.eip != 0x100u) return 1;
    }
    for (lib_u8 failure = 0u; failure != 4u; ++failure) {
        cpu_bus_fixture fixture;
        t_cpu before;
        const lib_u8 code[] = {0x0fu, 0x01u, 0xf0u};
        const lib_u8 gate = failure == 0u ? 0x80u :
            failure == 1u ? 0x0eu : 0x8eu;

        cpu_interrupt_prepare(&fixture, X86_CORE_CPU_PROFILE_80386, 0x0du, gate);
        lib_memory_copy(fixture.memory, code, sizeof(code));
        if (failure == 2u) fixture.memory[0x30du] = 0x7au;
        if (failure == 3u) fixture.cpu.data.ss.limit = 0x7ffeu;
        before = fixture.cpu;
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (!x86_core_cpu_is_shutdown(&fixture.execution) || fixture.faults ||
            fixture.execution.stop_requested ||
            fixture.cpu.data.eip != before.data.eip ||
            fixture.cpu.data.esp != before.data.esp ||
            fixture.cpu.data.eflags != before.data.eflags ||
            lib_memory_compare(&fixture.cpu.data.cs, &before.data.cs,
                sizeof(before.data.cs)) ||
            lib_memory_compare(&fixture.cpu.data.ss, &before.data.ss,
                sizeof(before.data.ss))) return 1;
    }
    return 0;
}

typedef struct cpu_entry_io_fixture {
    cpu_bus_fixture state;
    lib_u32 reject_address;
    lib_bool reject_write;
    lib_u64 accepted_ticks;
    lib_u32 accepted_cycles;
    lib_u32 buffered_write_issues;
    lib_u64 wait_ticks;
} cpu_entry_io_fixture;

static lib_status cpu_entry_io_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    cpu_entry_io_fixture *fixture = (cpu_entry_io_fixture *)opaque;
    if (!fixture->reject_write && address == fixture->reject_address)
        return LIB_STATUS_IO_ERROR;
    return cpu_bus_read(&fixture->state, address, destination, bytes,
        provenance, observe_only, reset_fetch, cache_control);
}

static lib_status cpu_entry_io_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    cpu_entry_io_fixture *fixture = (cpu_entry_io_fixture *)opaque;
    if (fixture->reject_write && address == fixture->reject_address)
        return LIB_STATUS_IO_ERROR;
    return cpu_bus_write(&fixture->state, address, source, bytes, provenance, cache_control);
}

static lib_bool cpu_entry_provider_failure(x86_core_cpu_profile profile)
{
    const lib_u8 code[] = {0x0fu,0x01u,0xf0u};
    const lib_u8 df_gate[] = {0u,1u,0x0bu,0u,0u,0x8eu,0u,0u};
    lib_u8 double_fault, write;

    for (double_fault = 0u; double_fault < 2u; ++double_fault)
    for (write = 0u; write < 2u; ++write) {
        cpu_entry_io_fixture fixture;
        x86_core_cpu_bus_provider bus = cpu_bus_provider;
        lib_u32 prior_flags = 0u;
        cpu_interrupt_prepare(&fixture.state, profile,
            13u, double_fault ? 0x80u : 0x8eu);
        fixture.reject_write = write != 0u;
        fixture.reject_address = write ? 0x7ff8u :
            0x400u + (double_fault ? 8u : 13u) * 8u;
        if (write && double_fault)
            lib_memory_copy(fixture.state.memory + 0x440u, df_gate, sizeof(df_gate));
        bus.read_memory = cpu_entry_io_read;
        bus.write_memory = cpu_entry_io_write;
        fixture.state.execution.bus = &bus;
        fixture.state.execution.bus_context = &fixture;
        lib_memory_copy(fixture.state.memory, code, sizeof(code));
        x86_core_cpu_execution_refresh(&fixture.state.execution);
        lib_memory_copy(&prior_flags, fixture.state.memory + 0x7fcu, sizeof(prior_flags));
        if (!fixture.state.execution.stop_requested || fixture.state.faults != 1u ||
            fixture.state.fault.exception_mask != VCPUINS_EXCEPT_CE ||
            fixture.state.fault.exception_code != fixture.reject_address ||
            fixture.state.execution.shutdown_requested ||
            (write && (fixture.state.writes == 0u || prior_flags == 0u))) {
            lib_c_printf("Entry IO profile=%u df=%u write=%u stop=%u fault=%u/%x/%x shutdown=%u rejected=%x\n",
                (unsigned)profile, double_fault, write, fixture.state.execution.stop_requested,
                fixture.state.faults, fixture.state.fault.exception_mask,
                fixture.state.fault.exception_code, fixture.state.execution.shutdown_requested,
                fixture.reject_address);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_u64 cpu_entry_accepted_cycle(void *opaque, x86_core_cpu_external_cycle_phase phase,
    x86_core_cpu_external_cycle_space space, lib_u32 address, lib_u8 bytes,
    lib_u8 write, x86_core_cpu_memory_access_provenance provenance)
{
    cpu_entry_io_fixture *fixture = opaque;
    (void)provenance;
    if (phase != X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT ||
        space != X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY) return 0u;
    const lib_u64 transfers = ((address & 3u) + bytes + 3u) / 4u;

    fixture->accepted_ticks += write ? 2u * transfers : 1u + transfers;
    ++fixture->accepted_cycles;
    if (write && (fixture->state.cpu.data.cr0 & VCPU_CR0_CD) == 0u)
        fixture->buffered_write_issues += (lib_u32)transfers;
    return fixture->wait_ticks;
}

static lib_bool cpu_entry_failed_prefix_timing(void)
{
    const lib_u8 code[] = {0x0fu, 0x01u, 0xf0u};
    const lib_u8 df_gate[] = {0u,1u,0x0bu,0u,0u,0x8eu,0u,0u};
    static const struct {lib_bool write; lib_u32 address;} failures[] = {
        {LIB_FALSE, 0x468u}, {LIB_FALSE, 0x308u},
        {LIB_TRUE, 0x7ffcu}, {LIB_TRUE, 0x7ff8u},
        {LIB_TRUE, 0x7ff4u}, {LIB_TRUE, 0x7ff0u}
    };

    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_80386;
         profile <= X86_CORE_CPU_PROFILE_80486SX; ++profile)
    for (lib_u8 nested = 0u; nested < 2u; ++nested)
    for (lib_u8 wait = 0u; wait <= 9u; wait += 9u)
    for (lib_size index = 0u; index < sizeof(failures) / sizeof(failures[0]); ++index) {
        cpu_entry_io_fixture fixture = {0};
        x86_core_cpu_bus_provider bus = cpu_bus_provider;
        x86_core_cpu_execution_context *context = &fixture.state.execution;
        const lib_bool sx = profile == X86_CORE_CPU_PROFILE_80486SX;

        cpu_interrupt_prepare(&fixture.state, profile, 13u, nested ? 0x80u : 0x8eu);
        fixture.reject_write = failures[index].write;
        fixture.reject_address = nested && index == 0u ? 0x440u : failures[index].address;
        fixture.wait_ticks = wait;
        if (nested) lib_memory_copy(fixture.state.memory + 0x440u, df_gate, sizeof(df_gate));
        bus.read_memory = cpu_entry_io_read;
        bus.write_memory = cpu_entry_io_write;
        context->bus = &bus;
        context->bus_context = &fixture;
        x86_core_cpu_execution_context_bind_external_cycle_provider(context,
            cpu_entry_accepted_cycle, &fixture);
        lib_memory_copy(fixture.state.memory, code, sizeof(code));
        x86_core_cpu_execution_refresh(context);
        if (!context->stop_requested || fixture.state.faults != 1u ||
            fixture.state.fault.exception_mask != VCPUINS_EXCEPT_CE ||
            fixture.state.fault.exception_code != fixture.reject_address ||
            context->timing_result.ticks != 0u || fixture.accepted_ticks == 0u ||
            /* The independent global bus credit retains actual service through
             * entry scopes. Failed-prefix reference work counts two clocks per
             * accepted buffered write, rather than its one issue clock. */
            context->timing_result.event_ticks != (sx ? context->bus_timing.service_credit +
                fixture.buffered_write_issues : 0u) ||
            fixture.state.instructions.data.timing_transfer_ticks != (sx ? fixture.accepted_ticks : 0u) ||
            fixture.state.instructions.data.timing_entry_transfer_ticks != (sx ? fixture.accepted_ticks : 0u) ||
            fixture.state.instructions.data.timing_external_wait_ticks !=
                (sx ? fixture.accepted_cycles * (lib_u64)wait : 0u)) {
            lib_c_printf("Failed entry prefix: profile=%u nested=%u wait=%u case=%zu event=%llu accepted=%llu credit=%llu buffered=%u cycles=%u mask=%x/%x\n",
                (lib_u32)profile, (lib_u32)nested, (lib_u32)wait, index,
                context->timing_result.event_ticks, fixture.accepted_ticks,
                context->bus_timing.service_credit, fixture.buffered_write_issues, fixture.accepted_cycles,
                fixture.state.fault.exception_mask, fixture.state.fault.exception_code);
            return LIB_FALSE;
        }
        x86_core_cpu_execution_finalize(context);
    }
    return LIB_TRUE;
}

static lib_bool cpu_external_error_matrix(x86_core_cpu_profile profile)
{
    const lib_u8 gdt[] = {0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0};
    const lib_u8 gp_gate[] = {0u,2u,8u,0u,0u,0x8eu,0u,0u};
    const lib_u32 expected[] = {0x102u,0x13u,0x183u};
    lib_u8 source;

    for (source = 0u; source < 3u; ++source) {
        cpu_bus_fixture state;
        lib_u32 error = 0u;
        cpu_bus_prepare(&state, profile);
        state.cpu.data.cr0 = VCPU_CR0_PE;
        state.cpu.data.cs.selector = 8u;
        state.cpu.data.ss.selector = 0x10u;
        state.cpu.data.eflags = 2u | VCPU_EFLAGS_IF;
        state.cpu.data.gdtr = (t_cpu_data_sreg){
            .flagValid = LIB_TRUE, .sregtype = SREG_GDTR, .base = 0x300u, .limit = 23u
        };
        state.cpu.data.idtr = (t_cpu_data_sreg){
            .flagValid = LIB_TRUE, .sregtype = SREG_IDTR, .base = 0x400u, .limit = 0x187u
        };
        lib_memory_copy(state.memory + 0x300u, gdt, sizeof(gdt));
        lib_memory_copy(state.memory + 0x400u + 13u * 8u, gp_gate, sizeof(gp_gate));
        state.memory[0x100u] = source ? 0x90u : 0xcdu;
        state.memory[0x101u] = 0x20u;
        if (source == 1u) x86_core_cpu_request_nmi(&state.execution);
        state.interrupt = source == 2u;
        x86_core_cpu_execution_refresh(&state.execution);
        lib_memory_copy(&error, state.memory + 0x6f0u, sizeof(error));
        if (state.execution.stop_requested || state.faults ||
            state.cpu.data.eip != 0x200u || state.cpu.data.sp != 0x6f0u ||
            error != expected[source] ||
            state.acknowledgements != (source == 2u ? 1u : 0u)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cpu_exception_pair_matrix(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_8088,
        X86_CORE_CPU_PROFILE_80186, X86_CORE_CPU_PROFILE_80286,
        X86_CORE_CPU_PROFILE_80386, X86_CORE_CPU_PROFILE_80486SX
    };
    static const struct { lib_u8 vector; lib_bool first_286; lib_u8 class_386; } rows[] = {
        {0u,LIB_TRUE,1u}, {1u,LIB_FALSE,0u}, {2u,LIB_FALSE,0u},
        {3u,LIB_FALSE,0u}, {4u,LIB_FALSE,0u}, {5u,LIB_FALSE,0u},
        {6u,LIB_FALSE,0u}, {7u,LIB_FALSE,0u}, {9u,LIB_FALSE,1u},
        {10u,LIB_TRUE,1u}, {11u,LIB_TRUE,1u}, {12u,LIB_TRUE,1u},
        {13u,LIB_TRUE,1u}, {14u,LIB_FALSE,2u}, {16u,LIB_FALSE,0u},
        {17u,LIB_FALSE,0u}
    };
    static const lib_bool table_386[3][3] = {
        {LIB_FALSE,LIB_FALSE,LIB_FALSE},
        {LIB_FALSE,LIB_TRUE,LIB_FALSE},
        {LIB_FALSE,LIB_TRUE,LIB_TRUE}
    };
    lib_size profile, first, second;
    lib_u32 count = 0u;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (first = 0u; first < sizeof(rows) / sizeof(rows[0]); ++first)
    for (second = 0u; second < sizeof(rows) / sizeof(rows[0]); ++second) {
        lib_bool expected = LIB_FALSE;
        if (profiles[profile] < X86_CORE_CPU_PROFILE_80486SX &&
            (rows[first].vector == 17u || rows[second].vector == 17u)) continue;
        /* Intel486 PRM 9.9.9: the 386 coprocessor overrun is not generated. */
        if (profiles[profile] == X86_CORE_CPU_PROFILE_80486SX &&
            (rows[first].vector == 9u || rows[second].vector == 9u)) continue;
        if (profiles[profile] == X86_CORE_CPU_PROFILE_80286) {
            if (rows[first].vector == 14u || rows[second].vector == 14u) continue;
            expected = rows[first].first_286;
        } else if (profiles[profile] >= X86_CORE_CPU_PROFILE_80386)
            expected = table_386[rows[first].class_386][rows[second].class_386];
        ++count;
        if (cpu_exception_requires_double_fault(profiles[profile],
                UINT32_C(1) << rows[first].vector,
                UINT32_C(1) << rows[second].vector) != expected) {
            lib_c_printf("Exception pair profile=%u first=%u second=%u expected=%u\n",
                (unsigned)profiles[profile], rows[first].vector, rows[second].vector, expected);
            return LIB_FALSE;
        }
    }
    lib_c_printf("Architectural exception-pair decisions=%u\n", (unsigned)count);
    return LIB_TRUE;
}

static lib_bool cpu_286_double_fault_task(lib_bool double_fault)
{
    const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0,
        0x2bu,0,0,8u,0,0x83u,0,0,
        0x2bu,0,0,9u,0,0x81u,0,0
    };
    const lib_u8 gate[] = {0,0,0x20u,0,0,0x85u,0,0};
    const lib_u16 incoming[] = {
        0,0,0,0,0,0,0,0x0100u,2u,0,0,0,0,0x9000u,0,0,0,0,8u,0x10u,0,0
    };
    const lib_u8 code[] = {0xf7u,0xf3u};
    const lib_u8 software_int[] = {0xcdu,8u};
    const lib_u16 sentinel = 0x5a5au;
    cpu_instruction_fixture state;
    lib_u16 error = 0xffffu, backlink = 0u;

    cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_80286);
    x86_core_cpu_execution_load_segment(&state.execution, &state.cpu.data.cs, 0x0200u);
    state.cpu.data.cs.selector = 8u;
    state.cpu.data.ss.selector = 0x10u;
    state.cpu.data.esp = 0x8000u;
    state.cpu.data.eflags = 2u;
    state.cpu.data.cr0 = VCPU_CR0_PE;
    state.cpu.data.gdtr = (t_cpu_data_sreg){
        .flagValid = LIB_TRUE, .sregtype = SREG_GDTR, .base = 0x300u,
        .limit = sizeof(gdt) - 1u
    };
    state.cpu.data.idtr = (t_cpu_data_sreg){
        .flagValid = LIB_TRUE, .sregtype = SREG_IDTR, .base = 0x400u, .limit = 0x6fu
    };
    state.cpu.data.tr = (t_cpu_data_sreg){
        .flagValid = LIB_TRUE, .sregtype = SREG_TR, .selector = 0x18u,
        .base = 0x800u, .limit = 0x2bu,
        .sys = {.type = VCPU_DESC_SYS_TYPE_TSS_16_BUSY}
    };
    lib_memory_copy(state.memory + 0x300u, gdt, sizeof(gdt));
    lib_memory_copy(state.memory + 0x400u + 8u * 8u, gate, sizeof(gate));
    lib_memory_copy(state.memory + 0x900u, incoming, sizeof(incoming));
    lib_memory_copy(state.memory + 0x8ffeu, &sentinel, sizeof(sentinel));
    lib_memory_copy(state.memory + 0x2000u, double_fault ? code : software_int, sizeof(code));
    x86_core_cpu_execution_refresh(&state.execution);
    lib_memory_copy(&error, state.memory + 0x8ffeu, sizeof(error));
    lib_memory_copy(&backlink, state.memory + 0x900u, sizeof(backlink));
    if (state.execution.stop_requested || state.fault.valid ||
        state.delivered_exception.valid != double_fault ||
        (double_fault && (state.delivered_exception.exception_mask != VCPUINS_EXCEPT_DF ||
            state.delivered_exception.exception_code != 0u)) ||
        state.cpu.data.eip != 0x0100u || state.cpu.data.sp != (double_fault ? 0x8ffeu : 0x9000u) ||
        state.cpu.data.tr.selector != 0x20u || error != (double_fault ? 0u : sentinel) || backlink != 0x18u) {
        lib_c_printf("286 DF task kind=%u stop=%u terminal=%x delivered=%x ip=%x sp=%x tr=%x code=%x backlink=%x\n",
            double_fault, state.execution.stop_requested, state.fault.exception_mask,
            state.delivered_exception.exception_mask, state.cpu.data.eip,
            state.cpu.data.sp, state.cpu.data.tr.selector, error, backlink);
        return LIB_FALSE;
    }
    if (!double_fault) lib_c_printf("286 DF task fixture: software INT control passed\n");
    return LIB_TRUE;
}

static lib_bool cpu_page_delivery_case(x86_core_cpu_profile profile, lib_u8 scenario)
{
    const lib_bool sx = profile == X86_CORE_CPU_PROFILE_80486SX;
    const lib_bool page_first = scenario >= 2u;
    const lib_bool missing_descriptor = scenario != 0u;
    const lib_bool double_fault = scenario == 2u || (sx && scenario == 3u);
    const lib_bool delivered_page = page_first || (sx && scenario == 1u);
    const lib_u32 expected_error = !sx && scenario == 3u ? 2u : 0u;
    const lib_u16 expected_cs = double_fault ? 8u :
        sx && scenario == 1u ? 0x1bu : page_first ? 0x1008u : 0x100bu;
    const lib_u16 expected_frame_cs = page_first ? 8u : 0x0bu;
    const lib_u8 descriptor[] = {0xffu,0xffu,0u,0x20u,0u,
        page_first ? 0x9au : 0xfau,0u,0u};
    const lib_u8 gp_gate[] = {0u,1u,0x0bu,0x10u,0u,0x8eu,0u,0u};
    const lib_u8 pf_gate[] = {0u,3u,scenario == 3u ? 8u : 0x1bu,
        scenario == 3u ? 0x10u : 0u,0u,0x8eu,0u,0u};
    const lib_u8 df_gate[] = {0u,4u,8u,0u,0u,0x8eu,0u,0u};
    const lib_u8 code[] = {0x0fu,0x01u,0xf0u};
    const lib_u8 store[] = {0xc6u,0x06u,0u,0x10u,0x5au};
    cpu_instruction_fixture state;
    const lib_u32 directory = 0x00011007u;
    lib_u32 frame[4] = {0};
    lib_u32 page;

    cpu_instruction_prepare(&state, profile);
    x86_core_cpu_execution_load_segment(&state.execution, &state.cpu.data.cs, 0x0200u);
    state.cpu.data.cs.selector = page_first ? 8u : 0x0bu;
    state.cpu.data.cs.dpl = page_first ? 0u : 3u;
    state.cpu.data.ss.selector = page_first ? 0x10u : 0x13u;
    state.cpu.data.ss.dpl = page_first ? 0u : 3u;
    state.cpu.data.ds.selector = 0x10u;
    state.cpu.data.ds.dpl = 0u;
    state.cpu.data.esp = 0x8000u;
    state.cpu.data.eflags = 2u;
    state.cpu.data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG;
    state.cpu.data.cr3 = 0x10000u;
    state.cpu.data.gdtr = (t_cpu_data_sreg){
        .flagValid = LIB_TRUE, .sregtype = SREG_GDTR, .base = 0x300u, .limit = 0x101fu
    };
    state.cpu.data.idtr = (t_cpu_data_sreg){
        .flagValid = LIB_TRUE, .sregtype = SREG_IDTR, .base = 0x400u, .limit = 0x87u
    };
    lib_memory_copy(state.memory + 0x10000u, &directory, sizeof(directory));
    for (page = 0u; page < 128u; ++page) {
        const lib_u32 entry = page == 1u && missing_descriptor ? 0u : page * 0x1000u + 7u;
        lib_memory_copy(state.memory + 0x11000u + page * 4u, &entry, sizeof(entry));
    }
    lib_memory_copy(state.memory + 0x308u, descriptor, sizeof(descriptor));
    lib_memory_copy(state.memory + 0x318u, descriptor, sizeof(descriptor));
    lib_memory_copy(state.memory + 0x1308u, descriptor, sizeof(descriptor));
    lib_memory_copy(state.memory + 0x400u + 13u * 8u, gp_gate, sizeof(gp_gate));
    if (scenario != 2u)
        lib_memory_copy(state.memory + 0x400u + 14u * 8u, pf_gate, sizeof(pf_gate));
    lib_memory_copy(state.memory + 0x400u + 8u * 8u, df_gate, sizeof(df_gate));
    lib_memory_copy(state.memory + 0x2000u, page_first ? store : code,
        page_first ? sizeof(store) : sizeof(code));
    x86_core_cpu_execution_refresh(&state.execution);
    lib_memory_copy(frame, state.memory + 0x7ff0u, sizeof(frame));
    if (state.execution.stop_requested || state.fault.valid ||
        !state.delivered_exception.valid ||
        state.delivered_exception.exception_mask != (double_fault ? VCPUINS_EXCEPT_DF :
            delivered_page ? VCPUINS_EXCEPT_PF : VCPUINS_EXCEPT_GP) ||
        state.delivered_exception.exception_code != expected_error ||
        state.delivered_exception.point.eip != 0u ||
        /* SX's actual missing target-descriptor page faults at linear 1308;
         * GP then PF is serial, while PF then PF requires DF (PRM 9.9.8). */
        state.cpu.data.cr2 != (sx && (scenario == 1u || scenario == 3u) ?
            0x1308u : page_first ? 0x1000u : 0u) ||
        state.cpu.data.eip != (double_fault ? 0x400u : delivered_page ? 0x300u : 0x100u) ||
        state.cpu.data.cs.selector != expected_cs ||
        state.cpu.data.sp != 0x7ff0u || frame[0] != expected_error || frame[1] != 0u ||
        frame[2] != expected_frame_cs) {
        lib_c_printf("Page delivery profile=%u scenario=%u stop=%u terminal=%x delivered=%x/%x point=%x cr2=%x cs=%x ip=%x sp=%x frame=%x/%x/%x\n",
            (unsigned)profile, scenario, state.execution.stop_requested, state.fault.exception_mask,
            state.delivered_exception.exception_mask, state.delivered_exception.exception_code,
            state.delivered_exception.point.eip,
            state.cpu.data.cr2, state.cpu.data.cs.selector,
            state.cpu.data.eip, state.cpu.data.sp, frame[0], frame[1], frame[2]);
        return LIB_FALSE;
    }
    if (!missing_descriptor) lib_c_printf("Serial GP/PF fixture: mapped GP control passed\n");
    return LIB_TRUE;
}

static lib_bool cpu_aam_return_matrix(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_8088,
        X86_CORE_CPU_PROFILE_80186, X86_CORE_CPU_PROFILE_80286,
        X86_CORE_CPU_PROFILE_80386
    };
    const lib_u16 vector[] = {0x0100u,0u};
    lib_size profile;
    lib_u8 prefix, zero;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (prefix = 0u; prefix < 2u; ++prefix)
    for (zero = 0u; zero < 2u; ++zero) {
        cpu_instruction_fixture state;
        const lib_u8 code[] = {0x26u,0xd4u,zero ? 0u : 0x0au};
        lib_u16 frame_ip = 0xffffu;
        const lib_u16 expected_ip = profiles[profile] < X86_CORE_CPU_PROFILE_80186 ?
            2u + prefix : 0u;
        cpu_instruction_prepare(&state, profiles[profile]);
        x86_core_cpu_execution_load_segment(&state.execution, &state.cpu.data.cs, 0x0200u);
        state.cpu.data.esp = 0x8000u;
        state.cpu.data.eax = 42u;
        state.cpu.data.eflags = 2u;
        lib_memory_copy(state.memory, vector, sizeof(vector));
        lib_memory_copy(state.memory + 0x2000u, code + (prefix ? 0u : 1u), 2u + prefix);
        x86_core_cpu_execution_refresh(&state.execution);
        lib_memory_copy(&frame_ip, state.memory + 0x7ffau, sizeof(frame_ip));
        if (state.execution.stop_requested || state.fault.valid) return LIB_FALSE;
        if (zero) {
            if (!state.delivered_exception.valid ||
                state.delivered_exception.exception_mask != VCPUINS_EXCEPT_DE ||
                state.delivered_exception.point.eip != 0u ||
                state.cpu.data.eip != 0x0100u || state.cpu.data.sp != 0x7ffau ||
                frame_ip != expected_ip) return LIB_FALSE;
        } else if (state.delivered_exception.valid ||
            state.cpu.data.eip != 2u + prefix || state.cpu.data.sp != 0x8000u ||
            state.cpu.data.ax != 0x0402u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cpu_divide_return_matrix(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_8088,
        X86_CORE_CPU_PROFILE_80186, X86_CORE_CPU_PROFILE_80286,
        X86_CORE_CPU_PROFILE_80386
    };
    static const struct {
        lib_u8 code[4];
        lib_u8 length;
        lib_u8 width;
        lib_bool signed_divide;
    } forms[] = {
        {{0xf6u,0xf3u,0u,0u},2u,1u,LIB_FALSE},
        {{0xf6u,0xfbu,0u,0u},2u,1u,LIB_TRUE},
        {{0xf7u,0xf3u,0u,0u},2u,2u,LIB_FALSE},
        {{0xf7u,0xfbu,0u,0u},2u,2u,LIB_TRUE},
        {{0xf6u,0x36u,0u,0x10u},4u,1u,LIB_FALSE},
        {{0xf6u,0x3eu,0u,0x10u},4u,1u,LIB_TRUE},
        {{0xf7u,0x36u,0u,0x10u},4u,2u,LIB_FALSE},
        {{0xf7u,0x3eu,0u,0x10u},4u,2u,LIB_TRUE}
    };
    const lib_u16 vector[] = {0x0100u,0u};
    lib_size profile, form;
    lib_u8 segment, wide, overflow;
    lib_u32 count = 0u, failures = 0u;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (form = 0u; form < sizeof(forms) / sizeof(forms[0]); ++form)
    for (segment = 0u; segment < 2u; ++segment)
    for (wide = 0u; wide < (profiles[profile] == X86_CORE_CPU_PROFILE_80386 &&
        forms[form].width == 2u ? 2u : 1u); ++wide)
    for (overflow = 0u; overflow < 2u; ++overflow) {
        cpu_instruction_fixture state;
        lib_u8 code[6] = {0};
        lib_u8 length = 0u;
        const lib_u8 width = wide ? 4u : forms[form].width;
        const lib_u32 divisor = overflow ? 1u : 0u;
        lib_u16 frame[3] = {0};
        const lib_u16 expected_ip = profiles[profile] < X86_CORE_CPU_PROFILE_80186 ?
            forms[form].length + (lib_u32)segment + wide : 0u;

        cpu_instruction_prepare(&state, profiles[profile]);
        x86_core_cpu_execution_load_segment(&state.execution,
            &state.cpu.data.cs, 0x0200u);
        state.cpu.data.esp = 0x8000u;
        state.cpu.data.eflags = 2u;
        state.cpu.data.ebx = divisor;
        state.cpu.data.eax = overflow ? (forms[form].signed_divide ?
            (width == 1u ? 0x80u : width == 2u ? 0x8000u : 0x80000000u) :
            (width == 1u ? 0x100u : 0u)) : 0x2222u;
        state.cpu.data.edx = overflow && !forms[form].signed_divide && width != 1u ? 1u : 0u;
        if (segment) code[length++] = 0x26u;
        if (wide) code[length++] = 0x66u;
        lib_memory_copy(code + length, forms[form].code, forms[form].length);
        length += forms[form].length;
        lib_memory_copy(state.memory, vector, sizeof(vector));
        lib_memory_copy(state.memory + 0x1000u, &divisor, width);
        lib_memory_copy(state.memory + 0x2000u, code, length);
        x86_core_cpu_execution_refresh(&state.execution);
        lib_memory_copy(frame, state.memory + 0x7ffau, sizeof(frame));
        ++count;
        if (state.execution.stop_requested || state.fault.valid ||
            !state.delivered_exception.valid ||
            state.delivered_exception.exception_mask != VCPUINS_EXCEPT_DE ||
            state.delivered_exception.point.cs != 0x0200u ||
            state.delivered_exception.point.eip != 0u ||
            state.cpu.data.eip != 0x0100u || state.cpu.data.sp != 0x7ffau ||
            frame[0] != expected_ip || frame[1] != 0x0200u) {
            if (failures < 6u) lib_c_printf("DE return profile=%u form=%u prefix=%u wide=%u overflow=%u frame=%x/%x terminal=%x\n",
                (unsigned)profiles[profile], (unsigned)form, segment, wide, overflow,
                frame[0], expected_ip, state.fault.exception_mask);
            ++failures;
        }
    }
    lib_c_printf("Divide return contexts=%u failures=%u\n", (unsigned)count, (unsigned)failures);
    return failures == 0u;
}

static lib_bool cpu_real_entry_new_cs(void)
{
    const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80286, X86_CORE_CPU_PROFILE_80386
    };
    const lib_u8 codes[][2] = {{0xcdu,0x20u}, {0x74u,0x7fu}};
    const lib_u16 vector[] = {0x0100u, 0u};

    for (lib_size profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (lib_u8 fault = 0u; fault < 2u; ++fault) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        lib_u16 frame[3] = {0};
        const lib_u8 id = fault ? 13u : 0x20u;

        cpu_instruction_prepare(&fixture, profiles[profile]);
        fixture.cpu.data.cs.limit = 0x7fu;
        fixture.cpu.data.esp = 0x8000u;
        fixture.cpu.data.eflags = VCPU_EFLAGS_ZF | 2u;
        lib_memory_copy(fixture.memory + id * 4u, vector, sizeof(vector));
        if (cpu_instruction_run(&fixture, codes[fault], sizeof(codes[fault]), &after) !=
                LIB_STATUS_OK || fixture.fault.valid ||
            x86_core_cpu_is_shutdown(&fixture.execution) ||
            after.data.eip != 0x0100u || after.data.cs.limit != 0xffffu ||
            after.data.esp != 0x7ffau || fixture.delivered_exception.valid != (fault != 0u) ||
            (fault && fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_GP)) {
            lib_c_printf("Real new-CS profile=%u fault=%u ip=%x limit=%x delivered=%x\n",
                (unsigned)profiles[profile], fault, after.data.eip,
                after.data.cs.limit, fixture.delivered_exception.exception_mask);
            return LIB_FALSE;
        }
        lib_memory_copy(frame, fixture.memory + 0x7ffau, sizeof(frame));
        if (frame[0] != (fault ? 0u : 2u) || frame[1] != 0u ||
            frame[2] != (VCPU_EFLAGS_ZF | 2u)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cpu_early_fixed_ivt(void)
{
    const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_8088,
        X86_CORE_CPU_PROFILE_80186
    };
    const lib_u8 code[] = {0xcdu, 0x20u};
    const lib_u16 vector[] = {0x0100u, 0u};

    for (lib_size index = 0u; index < sizeof(profiles) / sizeof(profiles[0]); ++index) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        lib_u16 frame[3] = {0};

        cpu_instruction_prepare(&fixture, profiles[index]);
        /* These private storage fields do not create an early-chip IDTR. */
        fixture.cpu.data.idtr.base = 0x0300u;
        fixture.cpu.data.idtr.limit = 0u;
        fixture.cpu.data.sp = 0x8000u;
        lib_memory_copy(fixture.memory + 0x20u * 4u, vector, sizeof(vector));
        if (cpu_instruction_run(&fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
            fixture.fault.valid || after.data.eip != 0x0100u ||
            after.data.sp != 0x7ffau || after.data.cs.selector != 0u)
            return LIB_FALSE;
        lib_memory_copy(frame, fixture.memory + 0x7ffau, sizeof(frame));
        if (frame[0] != sizeof(code) || frame[1] != 0u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cpu_shutdown_protected_recovery(void)
{
    const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80286, X86_CORE_CPU_PROFILE_80386
    };
    const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0
    };

    for (lib_size index = 0u; index < sizeof(profiles) / sizeof(profiles[0]); ++index)
    for (lib_u8 reject = 0u; reject < 2u; ++reject) {
        cpu_instruction_fixture fixture;
        const lib_u8 gate[] = {0u,1u,8u,0u,0u,
            reject ? 0x80u : (profiles[index] == X86_CORE_CPU_PROFILE_80286 ?
                0x86u : 0x8eu),0u,0u};

        cpu_instruction_prepare(&fixture, profiles[index]);
        lib_memory_copy(fixture.memory + 0x300u, gdt, sizeof(gdt));
        lib_memory_copy(fixture.memory + 0x400u + 2u * 8u, gate, sizeof(gate));
        fixture.cpu.data.gdtr.base = 0x300u;
        fixture.cpu.data.gdtr.limit = sizeof(gdt) - 1u;
        fixture.cpu.data.idtr.base = 0x400u;
        fixture.cpu.data.idtr.limit = 0x17u;
        fixture.cpu.data.cr0 |= VCPU_CR0_PE;
        x86_core_cpu_execution_load_segment(&fixture.execution, &fixture.cpu.data.cs, 8u);
        x86_core_cpu_execution_load_segment(&fixture.execution, &fixture.cpu.data.ss, 0x10u);
        fixture.cpu.data.esp = 0x8000u;
        fixture.cpu.data.eip = 0x20u;
        x86_core_cpu_execution_request_shutdown(&fixture.execution);
        if (!x86_core_cpu_request_nmi(&fixture.execution)) return LIB_FALSE;
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.fault.valid || fixture.execution.stop_requested ||
            !(fixture.cpu.data.cr0 & VCPU_CR0_PE) ||
            x86_core_cpu_is_shutdown(&fixture.execution) != (reject != 0u))
            return LIB_FALSE;
        if (!reject) {
            if (fixture.cpu.data.eip != 0x100u || fixture.cpu.data.cs.selector != 8u ||
                fixture.cpu.data.esp != 0x8000u -
                    (profiles[index] == X86_CORE_CPU_PROFILE_80286 ? 6u : 12u) ||
                !x86_core_cpu_execution_consume_instruction_fault_delivery(
                    &fixture.execution)) return LIB_FALSE;
        } else {
            fixture.memory[0x415u] = profiles[index] == X86_CORE_CPU_PROFILE_80286 ?
                0x86u : 0x8eu;
            if (!x86_core_cpu_request_nmi(&fixture.execution)) return LIB_FALSE;
            x86_core_cpu_execution_refresh(&fixture.execution);
            if (x86_core_cpu_is_shutdown(&fixture.execution) !=
                    (profiles[index] == X86_CORE_CPU_PROFILE_80286)) return LIB_FALSE;
        }
        x86_core_cpu_state_reset(&fixture.execution);
        if (x86_core_cpu_is_shutdown(&fixture.execution) ||
            (fixture.cpu.data.cr0 & VCPU_CR0_PE)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cpu_shutdown_recovery(void)
{
    const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80286, X86_CORE_CPU_PROFILE_80386
    };
    const lib_u16 nmi_vector[] = {0x0200u, 0u};

    for (lib_size index = 0u; index < sizeof(profiles) / sizeof(profiles[0]); ++index) {
        cpu_bus_fixture fixture;
        t_cpu before;

        cpu_bus_prepare(&fixture, profiles[index]);
        fixture.memory[0x100u] = 0x90u;
        fixture.cpu.data.eflags = VCPU_EFLAGS_IF | 2u;
        lib_memory_copy(fixture.memory + 8u, nmi_vector, sizeof(nmi_vector));
        before = fixture.cpu;
        x86_core_cpu_execution_request_shutdown(&fixture.execution);
        if (!x86_core_cpu_execution_consume_shutdown_request(&fixture.execution) ||
            !x86_core_cpu_is_shutdown(&fixture.execution)) return LIB_FALSE;
        fixture.interrupt = LIB_TRUE;
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (!x86_core_cpu_is_shutdown(&fixture.execution) || fixture.acknowledgements ||
            fixture.instruction_count || lib_memory_compare(&before, &fixture.cpu,
                sizeof(before))) return LIB_FALSE;
        if (!x86_core_cpu_request_nmi(&fixture.execution)) return LIB_FALSE;
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (x86_core_cpu_is_shutdown(&fixture.execution) || fixture.faults ||
            fixture.acknowledgements || fixture.cpu.data.eip != 0x0200u ||
            fixture.cpu.data.sp != before.data.sp - 6u ||
            !x86_core_cpu_execution_consume_instruction_fault_delivery(
                &fixture.execution)) return LIB_FALSE;
        /* A failed shutdown-NMI on 286 cannot be retried without RESET. */
        cpu_bus_prepare(&fixture, profiles[index]);
        fixture.cpu.data.idtr.limit = 0u;
        x86_core_cpu_execution_request_shutdown(&fixture.execution);
        if (!x86_core_cpu_request_nmi(&fixture.execution)) return LIB_FALSE;
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (!x86_core_cpu_is_shutdown(&fixture.execution) || fixture.faults ||
            fixture.execution.stop_requested) return LIB_FALSE;
        fixture.cpu.data.idtr.limit = 0x03ffu;
        lib_memory_copy(fixture.memory + 8u, nmi_vector, sizeof(nmi_vector));
        if (!x86_core_cpu_request_nmi(&fixture.execution)) return LIB_FALSE;
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (x86_core_cpu_is_shutdown(&fixture.execution) !=
                (profiles[index] == X86_CORE_CPU_PROFILE_80286))
            return LIB_FALSE;
        x86_core_cpu_execution_request_shutdown(&fixture.execution);
        x86_core_cpu_state_reset(&fixture.execution);
        if (x86_core_cpu_is_shutdown(&fixture.execution) ||
            x86_core_cpu_execution_consume_shutdown_request(&fixture.execution))
            return LIB_FALSE;
        x86_core_cpu_execution_request_shutdown(&fixture.execution);
        x86_core_cpu_state_initialize(&fixture.execution);
        if (x86_core_cpu_is_shutdown(&fixture.execution) ||
            x86_core_cpu_execution_consume_shutdown_request(&fixture.execution))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cpu_string_page_restart(x86_core_cpu_profile profile)
{
    static const lib_u8 opcodes[] = {0xa4u,0xa6u,0xaau,0xacu,0xaeu};
    static const lib_u8 widths[] = {1u,2u,4u};
    const lib_u32 arithmetic = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;

    for (lib_u32 context = 0u; context < 720u; ++context) {
        const lib_u8 mode = (lib_u8)(context % 5u);
        const lib_bool code32 = ((context / 5u) & 1u) != 0u;
        const lib_bool address32 = ((context / 10u) & 1u) != 0u;
        const lib_bool backward = ((context / 20u) & 1u) != 0u;
        const lib_bool repne = ((context / 40u) & 1u) != 0u;
        const lib_u8 width = widths[(context / 80u) % 3u];
        const lib_u32 completed = context / 240u;

        if (mode == 4u && code32) continue;
        for (lib_u8 operand = 0u; operand < 10u; ++operand) {
            const lib_u8 operation = (lib_u8)(operand / 2u);
            const lib_bool source_fault = (operand & 1u) == 0u;
            const lib_bool source_used = operation == 0u || operation == 1u || operation == 3u;
            const lib_bool destination_used = operation != 3u;
            const lib_bool compares = operation == 1u || operation == 4u;
            const lib_u32 boundary = source_fault ? 0xa000u : 0xd000u;
            const lib_u32 start = backward ? boundary - width + completed * width :
                boundary - completed * width;
            const lib_u32 source = source_fault ? start : 0xb100u;
            const lib_u32 destination = source_fault ? 0xe100u : start;
            const lib_u32 missing_page = backward ? boundary - 0x1000u : boundary;
            const lib_u32 pte_address = 0x11000u + (missing_page >> 12u) * 4u;
            const lib_u32 repaired_pte = missing_page | 7u;
            const lib_u32 directory = 0x11007u;
            const lib_u32 mask = width == 1u ? 0xffu : width == 2u ? 0xffffu : 0xffffffffu;
            const lib_u32 count_upper = address32 ? 0u : 0x56780000u;
            const lib_u32 index_upper = address32 ? 0u : 0x12340000u;
            const lib_u8 target_cpl = mode == 4u ? 0u : mode;
            lib_u8 handler[] = {0x66u,0x50u,0x1eu,0xb8u,
                (lib_u8)(0x10u | target_cpl),0u,0x8eu,0xd8u,
                0x66u,0x67u,0xc7u,0x05u,0,0,0,0,0,0,0,0,
                0x1fu,0x66u,0x58u,0x83u,0xc4u,0x04u,0x66u,0xcfu};
            const lib_u8 handler_cs[] = {0xffu,0xffu,0,0,0,
                (lib_u8)(0x9bu | (target_cpl << 5u)),0,0};
            const lib_u8 gate[] = {0u,3u,(lib_u8)(0x28u | target_cpl),0,0,0x8eu,0,0};
            lib_u8 code[5], bytes = 0u;
            lib_u32 frame[10], values[3], initial_destination[3];
            cpu_instruction_fixture fixture;
            t_cpu checkpoint;

            if ((source_fault && !source_used) || (!source_fault && !destination_used)) continue;
            cpu_instruction_prepare(&fixture, profile);
            cpu_instruction_prepare_protected_exception(&fixture, mode);
            fixture.cpu.data.cs.seg.exec.defsize = code32;
            fixture.cpu.data.ds.selector = (lib_u16)(0x10u | target_cpl);
            fixture.cpu.data.es.selector = (lib_u16)(0x10u | target_cpl);
            fixture.cpu.data.ds.dpl = mode == 4u ? 3u : mode;
            fixture.cpu.data.es.dpl = mode == 4u ? 3u : mode;
            if (mode == 4u) {
                fixture.cpu.data.cs.selector = 0u;
                fixture.cpu.data.ds.selector = 0u;
                fixture.cpu.data.es.selector = 0u;
                fixture.cpu.data.ss.selector = 0u;
            }
            fixture.cpu.data.eip = 0x1000u;
            fixture.cpu.data.esp = 0x8000u;
            fixture.cpu.data.esi = index_upper | source;
            fixture.cpu.data.edi = index_upper | destination;
            fixture.cpu.data.ecx = count_upper | 3u;
            fixture.cpu.data.eax = 0x10u;
            fixture.cpu.data.edx = 0x89abcdefu;
            fixture.cpu.data.ebx = 0x11223344u;
            fixture.cpu.data.ebp = 0x76543210u;
            fixture.cpu.data.eflags |= arithmetic | (backward ? VCPU_EFLAGS_DF : 0u) |
                (mode == 4u ? VCPU_EFLAGS_IOPL : 0u);
            fixture.cpu.data.cr0 |= VCPU_CR0_PG;
            fixture.cpu.data.cr3 = 0x10000u;
            fixture.cpu.data.gdtr.limit = 0x2fu;
            fixture.memory[0x20eu] = code32 ? 0x40u : 0u;
            fixture.memory[0x215u] = (lib_u8)(0x93u | (target_cpl << 5u));
            fixture.memory[0x216u] = 0x8fu;
            lib_memory_copy(fixture.memory + 0x228u, handler_cs, sizeof(handler_cs));
            lib_memory_copy(fixture.memory + 0x470u, gate, sizeof(gate));
            lib_memory_copy(fixture.memory + 0x10000u, &directory, sizeof(directory));
            for (lib_u32 page = 0u; page < 128u; ++page) {
                const lib_u32 entry = page * 0x1000u == missing_page ? 0u : page * 0x1000u + 7u;
                lib_memory_copy(fixture.memory + 0x11000u + page * 4u, &entry, sizeof(entry));
            }
            for (lib_u32 element = 0u; element < 3u; ++element) {
                const lib_u32 delta = backward ? 0u - element * width : element * width;
                values[element] = compares ? 0x10u : 0x10203040u + element;
                initial_destination[element] = compares ? (repne ? 0x0fu : 0x10u) : 0xa5a5a5a5u;
                lib_memory_copy(fixture.memory + (source + delta), &values[element], width);
                lib_memory_copy(fixture.memory + (destination + delta), &initial_destination[element], width);
            }
            if (width == 1u ? repne : code32 != (width == 4u)) code[bytes++] = 0x66u;
            if (code32 != address32) code[bytes++] = 0x67u;
            code[bytes++] = repne ? 0xf2u : 0xf3u;
            code[bytes++] = (lib_u8)(opcodes[operation] + (width != 1u));
            lib_memory_copy(fixture.memory + 0x1000u, code, bytes);
            lib_memory_copy(handler + 12u, &pte_address, sizeof(pte_address));
            lib_memory_copy(handler + 16u, &repaired_pte, sizeof(repaired_pte));
            lib_memory_copy(fixture.memory + 0x300u, handler, sizeof(handler));
            const lib_u32 initial_flags = fixture.cpu.data.eflags;
            checkpoint = fixture.cpu;
            fixture.cpu.data.ecx = count_upper;
            t_cpu zero_count = fixture.cpu;
            zero_count.data.eip += bytes;
            x86_core_cpu_execution_refresh(&fixture.execution);
            if (fixture.execution.stop_requested || fixture.fault.valid ||
                fixture.delivered_exception.valid ||
                lib_memory_compare(&zero_count, &fixture.cpu, sizeof(zero_count)) != 0)
                goto failure;
            fixture.cpu = checkpoint;
            x86_core_cpu_execution_invalidate_prefetch(&fixture.execution);
            for (lib_u32 step = 0u; step < completed; ++step) {
                x86_core_cpu_execution_refresh(&fixture.execution);
                const lib_u32 delta = backward ? 0u - (step + 1u) * width : (step + 1u) * width;
                if (fixture.execution.stop_requested || fixture.fault.valid ||
                    fixture.delivered_exception.valid || fixture.cpu.data.eip != 0x1000u ||
                    fixture.cpu.data.ecx != count_upper + 2u - step ||
                    fixture.cpu.data.esi != (index_upper | source) + (source_used ? delta : 0u) ||
                    fixture.cpu.data.edi != (index_upper | destination) + (destination_used ? delta : 0u) ||
                    fixture.cpu.data.eax != (operation == 3u ? values[step] & mask : 0x10u) ||
                    fixture.execution.repeat_compare_flags_valid !=
                        (compares && profile == X86_CORE_CPU_PROFILE_80486SX))
                    goto failure;
            }
            checkpoint = fixture.cpu;
            x86_core_cpu_execution_refresh(&fixture.execution);
            if (fixture.execution.stop_requested || fixture.fault.valid ||
                !fixture.delivered_exception.valid || x86_core_cpu_is_shutdown(&fixture.execution) ||
                fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_PF ||
                fixture.delivered_exception.exception_code !=
                    ((mode >= 3u ? 4u : 0u) | (!source_fault && (operation == 0u || operation == 2u) ? 2u : 0u)) ||
                fixture.delivered_exception.point.eip != 0x1000u ||
                fixture.cpu.data.cr2 != (backward ? boundary - width : boundary) ||
                fixture.cpu.data.eip != 0x300u || fixture.execution.repeat_compare_flags_valid ||
                fixture.cpu.data.eax != checkpoint.data.eax ||
                fixture.cpu.data.ecx != checkpoint.data.ecx ||
                fixture.cpu.data.esi != checkpoint.data.esi || fixture.cpu.data.edi != checkpoint.data.edi)
                goto failure;
            lib_memory_copy(frame, fixture.memory + fixture.cpu.data.ss.base +
                fixture.cpu.data.esp, sizeof(frame));
            if (compares && context == 240u)
                lib_c_printf("STRING-COMPARE-CHECKPOINT profile=%u operation=%u operand=%u initial=%x last=%x fault=%x\n",
                    profile, operation, operand, initial_flags & arithmetic,
                    checkpoint.data.eflags & arithmetic, frame[3] & arithmetic);
            const lib_u32 expected_flags = compares &&
                profile == X86_CORE_CPU_PROFILE_80486SX ? initial_flags : checkpoint.data.eflags;
            if (frame[0] != fixture.delivered_exception.exception_code || frame[1] != 0x1000u ||
                frame[2] != checkpoint.data.cs.selector ||
                frame[3] != (expected_flags | VCPU_EFLAGS_RF) ||
                fixture.delivered_exception.eflags != expected_flags)
                goto failure;
            for (lib_u32 element = 0u; element < 3u; ++element) {
                const lib_u32 delta = backward ? 0u - element * width : element * width;
                lib_u32 actual = 0u;
                const lib_u32 expected = element < completed && operation == 0u ? values[element] :
                    element < completed && operation == 2u ? 0x10u : initial_destination[element];
                lib_memory_copy(&actual, fixture.memory + (destination + delta), width);
                if (actual != (expected & mask)) goto failure;
                lib_memory_copy(&actual, fixture.memory + (source + delta), width);
                if (actual != (values[element] & mask)) goto failure;
            }
            fixture.delivered_exception.valid = LIB_FALSE;
            for (lib_u8 step = 0u; step < 9u; ++step) {
                x86_core_cpu_execution_refresh(&fixture.execution);
                if (fixture.execution.stop_requested || fixture.fault.valid ||
                    fixture.delivered_exception.valid || fixture.execution.repeat_compare_flags_valid)
                    goto failure;
            }
            if (fixture.cpu.data.eip != 0x1000u || fixture.cpu.data.esp != 0x8000u ||
                fixture.cpu.data.ecx != checkpoint.data.ecx ||
                fixture.cpu.data.eax != checkpoint.data.eax ||
                fixture.cpu.data.esi != checkpoint.data.esi || fixture.cpu.data.edi != checkpoint.data.edi)
                goto failure;
            lib_memory_copy(frame, fixture.memory + pte_address, sizeof(repaired_pte));
            if (frame[0] != repaired_pte) goto failure;
            for (lib_u32 step = completed; step < 3u; ++step) {
                x86_core_cpu_execution_refresh(&fixture.execution);
                if (fixture.execution.stop_requested || fixture.fault.valid ||
                    fixture.delivered_exception.valid) goto failure;
            }
            if (fixture.cpu.data.eip != 0x1000u + bytes || fixture.cpu.data.ecx != count_upper ||
                fixture.execution.repeat_compare_flags_valid ||
                fixture.cpu.data.eax != (operation == 3u ? values[2] & mask : 0x10u) ||
                fixture.cpu.data.edx != 0x89abcdefu || fixture.cpu.data.ebx != 0x11223344u ||
                fixture.cpu.data.ebp != 0x76543210u || fixture.cpu.data.esp != 0x8000u ||
                fixture.cpu.data.esi != ((index_upper | source) +
                    (source_used ? (backward ? 0u - 3u * width : 3u * width) : 0u)) ||
                fixture.cpu.data.edi != ((index_upper | destination) +
                    (destination_used ? (backward ? 0u - 3u * width : 3u * width) : 0u))) goto failure;
            for (lib_u32 element = 0u; element < 3u; ++element) {
                const lib_u32 delta = backward ? 0u - element * width : element * width;
                const lib_u32 expected = operation == 0u ? values[element] :
                    operation == 2u ? 0x10u : initial_destination[element];
                lib_u32 actual = 0u;
                lib_memory_copy(&actual, fixture.memory + (destination + delta), width);
                if (actual != (expected & mask)) goto failure;
                lib_memory_copy(&actual, fixture.memory + (source + delta), width);
                if (actual != (values[element] & mask)) goto failure;
            }
            continue;
failure:
            lib_c_printf("STRING-PAGE-RESTART profile=%u context=%u operand=%u ip=%x flags=%x cr2=%x count=%x fault=%x/%x stop=%u\n",
                profile, context, operand, fixture.cpu.data.eip, fixture.cpu.data.eflags,
                fixture.cpu.data.cr2, fixture.cpu.data.ecx, fixture.delivered_exception.exception_mask,
                fixture.delivered_exception.exception_code, fixture.execution.stop_requested);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool cpu_compare_fault_flags(void)
{
    for (lib_u8 scenario = 0u; scenario < 8u; ++scenario)
    for (lib_u8 form = 0u; form < 3u; ++form)
    for (lib_u8 repne = 0u; repne < 2u; ++repne) {
        const lib_bool real = scenario >= 6u;
        const lib_bool alignment = scenario == 4u || scenario == 5u;
        const lib_bool scan = scenario == 3u || scenario == 5u || scenario == 7u;
        const lib_bool source_fault = scenario == 1u || scenario == 2u;
        const lib_bool stack = scenario == 2u;
        const lib_u8 width = (lib_u8)(1u << form);
        const lib_u8 vector = alignment ? 17u : stack ? 12u : 13u;
        const lib_u32 value = 0x10u;
        const lib_u32 right = repne ? 0x0fu : value;
        lib_u8 code[4], bytes = 0u;
        lib_u32 frame[4] = {0u};
        lib_u16 real_frame[3] = {0u};
        cpu_instruction_fixture fixture;

        if (alignment && width == 1u) continue;
        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        if (!real) cpu_instruction_prepare_protected_exception(&fixture, alignment ? 3u : 0u);
        else fixture.cpu.data.esp = 0x800u;
        fixture.cpu.data.eip = 0x1000u;
        fixture.cpu.data.eax = value;
        fixture.cpu.data.esi = 0x3000u;
        fixture.cpu.data.edi = real ? 0x10000u - width : alignment ? 0x4001u : 0x4000u;
        fixture.cpu.data.ecx = 3u;
        fixture.cpu.data.eflags |= VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_SF |
            VCPU_EFLAGS_OF | (alignment ? VCPU_EFLAGS_AC : 0u);
        fixture.cpu.data.ds.dpl = fixture.cpu.data.es.dpl = alignment ? 3u : 0u;
        if (!real) fixture.cpu.data.ds.selector = fixture.cpu.data.es.selector = alignment ? 0x13u : 0x10u;
        if (alignment) fixture.cpu.data.cr0 |= VCPU_CR0_AM;
        else if (stack) fixture.cpu.data.ss.limit = 0x3000u + width - 1u;
        else if (source_fault) fixture.cpu.data.ds.limit = 0x3000u + width - 1u;
        else if (!real) fixture.cpu.data.es.limit = 0x4000u + width - 1u;
        if (!real) {
            fixture.cpu.data.idtr.limit = 0x8fu;
            lib_memory_copy(fixture.memory + 0x400u + vector * 8u,
                fixture.memory + 0x468u, 8u);
        } else {
            const lib_u16 handler[] = {0x300u,0u};
            lib_memory_copy(fixture.memory + vector * 4u, handler, sizeof(handler));
        }
        lib_memory_copy(fixture.memory + 0x3000u, &value, width);
        lib_memory_copy(fixture.memory + fixture.cpu.data.edi, &right, width);
        code[bytes++] = repne ? 0xf2u : 0xf3u;
        if (width == 4u) code[bytes++] = 0x66u;
        if (real) code[bytes++] = 0x67u;
        if (stack) code[bytes++] = 0x36u;
        code[bytes++] = (lib_u8)((scan ? 0xaeu : 0xa6u) | (width != 1u));
        lib_memory_copy(fixture.memory + 0x1000u, code, bytes);
        const lib_u32 initial_flags = fixture.cpu.data.eflags;
        if (!alignment) {
            x86_core_cpu_execution_refresh(&fixture.execution);
            if (fixture.delivered_exception.valid || fixture.fault.valid ||
                fixture.cpu.data.eip != 0x1000u || fixture.cpu.data.ecx != 2u ||
                !fixture.execution.repeat_compare_flags_valid) return LIB_FALSE;
        }
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (real) lib_memory_copy(real_frame, fixture.memory + fixture.cpu.data.sp, sizeof(real_frame));
        else lib_memory_copy(frame, fixture.memory + fixture.cpu.data.ss.base +
            fixture.cpu.data.esp, sizeof(frame));
        if (fixture.execution.stop_requested || fixture.fault.valid ||
            x86_core_cpu_is_shutdown(&fixture.execution) || fixture.execution.repeat_compare_flags_valid ||
            !fixture.delivered_exception.valid || fixture.delivered_exception.exception_mask != (1u << vector) ||
            fixture.delivered_exception.exception_code != 0u ||
            fixture.delivered_exception.eflags != initial_flags ||
            fixture.delivered_exception.point.eip != 0x1000u || fixture.cpu.data.eip != 0x300u ||
            (!real && (frame[0] != 0u || frame[1] != 0x1000u || frame[3] != (initial_flags | VCPU_EFLAGS_RF))) ||
            (real && (real_frame[0] != 0x1000u || real_frame[1] != 0u || real_frame[2] != initial_flags)) ||
            fixture.cpu.data.ecx != (alignment ? 3u : 2u) ||
            fixture.cpu.data.esi != (scan || alignment ? 0x3000u : 0x3000u + width) ||
            fixture.cpu.data.edi != (real ? 0x10000u : alignment ? 0x4001u : 0x4000u + width)) {
            lib_c_printf("COMPARE-FAULT-FLAGS scenario=%u form=%u repne=%u mask=%x flags=%x/%x frame=%x\n",
                scenario, form, repne, fixture.delivered_exception.exception_mask,
                fixture.delivered_exception.eflags, initial_flags, frame[3]);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool cpu_compare_flags_lifetime(void)
{
    const lib_u8 code[] = {0xf3u,0xa6u};
    const lib_u32 flags = 2u | VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_OF;

    for (lib_u8 scenario = 0u; scenario < 7u; ++scenario) {
        cpu_instruction_fixture fixture;
        x86_core_cpu_execution_context *context = &fixture.execution;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.ecx = 2u;
        fixture.cpu.data.esi = 0x500u;
        fixture.cpu.data.edi = 0x600u;
        fixture.cpu.data.eflags = flags;
        x86_core_cpu_execution_invalidate_prefetch(context);
        if (cpu_instruction_run(&fixture, code, sizeof(code), &fixture.cpu) != LIB_STATUS_OK ||
            !context->repeat_compare_flags_valid || context->repeat_compare_flags != flags ||
            fixture.cpu.data.ecx != 1u || fixture.delivered_exception.valid) return LIB_FALSE;
        {
            const x86_core_cpu_execution_context before = *context;
            x86_core_cpu_instruction_lexeme lexeme;
            if (!x86_core_cpu_execution_preview_lexeme(context, &lexeme) ||
                !lexeme.available || lexeme.byte_count != sizeof(code) ||
                lib_memory_compare(&before, context, sizeof(before)) != 0) return LIB_FALSE;
            x86_core_cpu_execution_request_debug_pause(context);
            if (!x86_core_cpu_execution_consume_debug_pause_request(context) ||
                !context->repeat_compare_flags_valid) return LIB_FALSE;
        }
        if (scenario == 0u) {
            const t_cpu before = fixture.cpu;
            if (x86_core_cpu_set_stop_clock(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
            x86_core_cpu_execution_refresh(context);
            if (!context->stop_granted || !context->repeat_compare_flags_valid ||
                lib_memory_compare(&before, &fixture.cpu, sizeof(before)) != 0) return LIB_FALSE;
            if (x86_core_cpu_set_clock_running(context, LIB_FALSE) != LIB_STATUS_OK) return LIB_FALSE;
            x86_core_cpu_execution_refresh(context);
            if (!context->repeat_compare_flags_valid) return LIB_FALSE;
            if (x86_core_cpu_set_clock_running(context, LIB_TRUE) != LIB_STATUS_OK ||
                x86_core_cpu_set_stop_clock(context, LIB_FALSE) != LIB_STATUS_OK) return LIB_FALSE;
            x86_core_cpu_execution_refresh(context);
        } else if (scenario == 1u) {
            lib_u32 value = 0u;
            x86_core_cpu_debug_register_patch patch = {
                .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR4)};
            patch.values[X86_CORE_CPU_DEBUG_CR4] = 0xffffffffu;
            if (x86_core_cpu_debug_read_register(context, X86_CORE_CPU_DEBUG_EFLAGS, &value) != LIB_STATUS_OK ||
                !context->repeat_compare_flags_valid ||
                x86_core_cpu_debug_patch_registers(context, &patch) != LIB_STATUS_INVALID_ARGUMENT ||
                !context->repeat_compare_flags_valid) return LIB_FALSE;
            patch.mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS);
            patch.values[X86_CORE_CPU_DEBUG_EFLAGS] = flags;
            if (x86_core_cpu_debug_patch_registers(context, &patch) != LIB_STATUS_OK) return LIB_FALSE;
        } else if (scenario == 2u) x86_core_cpu_state_reset(context);
        else if (scenario == 3u) {
            if (x86_core_cpu_state_soft_reset(context) != LIB_STATUS_OK) return LIB_FALSE;
        } else if (scenario == 4u || scenario == 5u) {
            const x86_core_cpu_entry_plan_state state = {.ip = 0x100u,.sp = 0x700u};
            x86_core_cpu_prepared_entry *entry = LIB_NULL;
            if (x86_core_cpu_prepare_entry(context, &state, &entry) != LIB_STATUS_OK) return LIB_FALSE;
            x86_core_cpu_finish_entry(entry, LIB_FALSE);
            if (!context->repeat_compare_flags_valid ||
                x86_core_cpu_prepare_entry(context, &state, &entry) != LIB_STATUS_OK) return LIB_FALSE;
            x86_core_cpu_finish_entry(entry, scenario == 4u);
            if (scenario == 5u) {
                if (!context->repeat_compare_flags_valid) return LIB_FALSE;
                x86_core_cpu_execution_refresh(context);
            }
        } else x86_core_cpu_execution_refresh(context);
        if (context->repeat_compare_flags_valid) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cpu_sx_reserved_escaped_slots(void)
{
    /* Original opcode map plus the selected stepping's historical-form
     * dispositions: MSR, ICE/LOADALL and A-step forms are not admitted. */
    static const lib_u8 ranges[][2] = {
        {0x04u, 0x05u}, {0x07u, 0x07u}, {0x0au, 0x0fu}, {0x14u, 0x1fu},
        {0x25u, 0x25u}, {0x27u, 0x7fu}, {0xa6u, 0xa7u}, {0xaeu, 0xaeu},
        {0xb8u, 0xb9u}, {0xc2u, 0xc7u}, {0xd0u, 0xffu}
    };
    lib_size range;
    lib_u16 opcode;
    lib_u8 code32, prefix, tail;

    for (range = 0u; range < sizeof(ranges) / sizeof(ranges[0]); ++range)
    for (opcode = ranges[range][0]; opcode <= ranges[range][1]; ++opcode)
    for (code32 = 0u; code32 < 2u; ++code32)
    for (prefix = 0u; prefix < 2u; ++prefix)
    for (tail = 0u; tail < 2u; ++tail) {
        const lib_u8 code[] = {0x66u, 0x0fu, (lib_u8)opcode,
            tail ? 0xc0u : 0x06u, 0u, 0u, 0u};
        const lib_u8 start = prefix ? 0u : 1u;
        cpu_instruction_fixture state;
        x86_core_cpu_instruction_lexeme lexeme;
        lib_bool passed;

        cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_80486SX);
        state.cpu.data.cs.seg.exec.defsize = code32;
        state.cpu.data.eax = 0x12345678u;
        state.cpu.data.ecx = 0x87654321u;
        state.cpu.data.eflags = 2u | VCPU_EFLAGS_CF | VCPU_EFLAGS_DF | VCPU_EFLAGS_IF;
        passed = !x86_core_cpu_instruction_lexeme_scan(code + start,
            sizeof(code) - start, X86_CORE_CPU_PROFILE_80486SX, code32, &lexeme) &&
            !lexeme.available && cpu_instruction_expect_real_fault(&state,
                code + start, sizeof(code) - start, 6u);
        x86_core_cpu_execution_finalize(&state.execution);
        if (!passed) {
            lib_c_printf("SX reserved 0F%02X code32=%u prefix=%u tail=%u\n",
                opcode, code32, prefix, tail);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool cpu_sx_reserved_modrm_forms(void)
{
    static const struct {
        lib_u8 opcode;
        lib_bool escaped;
        lib_u8 rejected_extensions;
        lib_bool register_only;
    } forms[] = {
        {0x8fu, LIB_FALSE, 0xfeu, LIB_FALSE},
        {0xc6u, LIB_FALSE, 0xfeu, LIB_FALSE},
        {0xc7u, LIB_FALSE, 0xfeu, LIB_FALSE},
        {0xfeu, LIB_FALSE, 0xfcu, LIB_FALSE},
        {0xffu, LIB_FALSE, 0x80u, LIB_FALSE},
        {0x8cu, LIB_FALSE, 0xc0u, LIB_FALSE},
        {0x8eu, LIB_FALSE, 0xc2u, LIB_FALSE},
        {0x00u, LIB_TRUE, 0xc0u, LIB_FALSE},
        {0x01u, LIB_TRUE, 0x20u, LIB_FALSE},
        {0xbau, LIB_TRUE, 0x0fu, LIB_FALSE},
        {0x62u, LIB_FALSE, 0xffu, LIB_TRUE},
        {0x8du, LIB_FALSE, 0xffu, LIB_TRUE},
        {0xc4u, LIB_FALSE, 0xffu, LIB_TRUE},
        {0xc5u, LIB_FALSE, 0xffu, LIB_TRUE},
        {0xffu, LIB_FALSE, 0x28u, LIB_TRUE},
        {0x01u, LIB_TRUE, 0x8fu, LIB_TRUE},
        {0xb2u, LIB_TRUE, 0xffu, LIB_TRUE},
        {0xb4u, LIB_TRUE, 0xffu, LIB_TRUE},
        {0xb5u, LIB_TRUE, 0xffu, LIB_TRUE}
    };
    lib_size form;
    lib_u16 modrm;
    lib_u8 code32, prefix;
    lib_bool reported[sizeof(forms) / sizeof(forms[0])] = {LIB_FALSE};
    lib_bool all_passed = LIB_TRUE;

    for (form = 0u; form < sizeof(forms) / sizeof(forms[0]); ++form)
    for (modrm = 0u; modrm < 256u; ++modrm)
    for (code32 = 0u; code32 < 2u; ++code32)
    for (prefix = 0u; prefix < 2u; ++prefix) {
        lib_u8 code[15] = {0u};
        lib_u8 bytes = 0u;
        cpu_instruction_fixture state;
        x86_core_cpu_instruction_lexeme lexeme;
        lib_bool passed;

        if (!(forms[form].rejected_extensions & (1u << ((modrm >> 3u) & 7u))) ||
            (forms[form].register_only && modrm < 0xc0u)) continue;
        if (prefix) code[bytes++] = 0x66u;
        if (forms[form].escaped) code[bytes++] = 0x0fu;
        code[bytes++] = forms[form].opcode;
        code[bytes++] = (lib_u8)modrm;
        cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_80486SX);
        state.cpu.data.cs.seg.exec.defsize = code32;
        state.cpu.data.eax = 0x12345678u;
        state.cpu.data.ecx = 0x87654321u;
        state.cpu.data.eflags = 2u | VCPU_EFLAGS_CF | VCPU_EFLAGS_DF | VCPU_EFLAGS_IF;
        passed = !x86_core_cpu_instruction_lexeme_scan(code, sizeof(code),
            X86_CORE_CPU_PROFILE_80486SX, code32, &lexeme) && !lexeme.available &&
            cpu_instruction_expect_real_fault(&state, code, sizeof(code), 6u);
        x86_core_cpu_execution_finalize(&state.execution);
        if (!passed && !reported[form]) {
            lib_c_printf("SX reserved form=%zu modrm=%02X code32=%u prefix=%u\n",
                form, modrm, code32, prefix);
            reported[form] = LIB_TRUE;
        }
        all_passed = all_passed && passed;
    }
    return all_passed;
}

typedef struct cpu_event_timing_fixture {
    cpu_instruction_fixture instruction;
    lib_bool pending;
    lib_bool reject_acknowledge;
    lib_u8 vector;
    lib_u8 acknowledgements;
    lib_u8 gate_reads;
    lib_u8 code_reads;
    lib_u8 stack_reads;
    lib_u32 uncached_start;
    lib_u32 uncached_limit;
    lib_u32 reject_address;
    lib_bool reject_write;
    lib_u32 table_reads;
    lib_u32 table_writes;
    lib_u64 accepted_ticks;
    lib_u64 fetch_ticks;
    lib_u64 interrupted_ticks;
    lib_u64 wait_ticks;
} cpu_event_timing_fixture;

static lib_bool cpu_event_timing_pending(void *opaque)
{
    return ((cpu_event_timing_fixture *)opaque)->pending;
}

static lib_status cpu_event_timing_acknowledge(void *opaque, lib_u8 *vector)
{
    cpu_event_timing_fixture *fixture = opaque;

    ++fixture->acknowledgements;
    fixture->interrupted_ticks = x86_core_cpu_capture_timing(&fixture->instruction.execution).ticks;
    if (fixture->reject_acknowledge) return LIB_STATUS_IO_ERROR;
    fixture->pending = LIB_FALSE;
    *vector = fixture->vector;
    return LIB_STATUS_OK;
}

static lib_status cpu_event_descriptor_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    cpu_event_timing_fixture *fixture = opaque;

    if (!observe_only && fixture->reject_address != 0u && !fixture->reject_write &&
        address <= fixture->reject_address &&
        (lib_u64)address + bytes > fixture->reject_address) return LIB_STATUS_IO_ERROR;
    if (!observe_only && provenance == X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ)
        ++fixture->table_reads;
    if (!observe_only && provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) {
        if (address == 0x400u + fixture->vector * 8u) ++fixture->gate_reads;
        if (address == 0x208u) ++fixture->code_reads;
        if (address == 0x210u) ++fixture->stack_reads;
    }
    return cpu_instruction_read(&fixture->instruction, address, destination, bytes,
        provenance, observe_only, reset_fetch, cache_control);
}

static lib_bool cpu_event_descriptor_once(void)
{
    const lib_u8 nop = 0x90u;

    for (lib_u8 mode = 0u; mode < 5u; ++mode) {
        cpu_event_timing_fixture fixture = {0};
        x86_core_cpu_bus_provider bus = cpu_instruction_bus;
        cpu_instruction_fixture *instruction = &fixture.instruction;
        const lib_bool inner = mode >= 2u;
        const lib_bool gate32 = mode == 1u || mode >= 3u;

        bus.read_memory = cpu_event_descriptor_read;
        bus.interrupt_pending = cpu_event_timing_pending;
        bus.acknowledge_interrupt = cpu_event_timing_acknowledge;
        cpu_instruction_prepare_with_bus(instruction, X86_CORE_CPU_PROFILE_80486SX, &bus, &fixture);
        cpu_instruction_prepare_protected_exception(instruction, inner ? 4u : 0u);
        if (mode != 4u) instruction->cpu.data.eflags &= ~VCPU_EFLAGS_VM;
        instruction->cpu.data.eflags |= VCPU_EFLAGS_IF;
        instruction->cpu.data.idtr.limit = 0x187u;
        lib_memory_copy(instruction->memory + 0x580u, instruction->memory + 0x468u, 8u);
        instruction->memory[0x585u] = gate32 ? 0x8eu : 0x86u;
        instruction->memory[0] = nop;
        fixture.pending = LIB_TRUE;
        fixture.vector = 0x30u;
        x86_core_cpu_execution_refresh(&instruction->execution);
        if (instruction->fault.valid || instruction->execution.stop_requested ||
            fixture.acknowledgements != 1u || fixture.gate_reads != 1u || fixture.code_reads != 1u ||
            fixture.stack_reads != (inner ? 1u : 0u) ||
            instruction->cpu.data.eip != 0x300u || instruction->cpu.data.cs.dpl != 0u) {
            lib_c_printf("Event descriptor ownership: mode=%u gate=%u code=%u eip=%x fault=%x\n",
                (lib_u32)mode, (lib_u32)fixture.gate_reads, (lib_u32)fixture.code_reads,
                instruction->cpu.data.eip, instruction->fault.exception_mask);
            return LIB_FALSE;
        }
        x86_core_cpu_execution_finalize(&instruction->execution);
    }
    return LIB_TRUE;
}

static lib_status cpu_entry_cache_query(void *opaque, lib_u32 address,
    x86_core_cpu_cache_query query, x86_core_cpu_cache_memory *out_memory)
{
    const cpu_event_timing_fixture *fixture = opaque;
    *out_memory = (x86_core_cpu_cache_memory){.address = address,
        .fill_enabled = query == X86_CORE_CPU_CACHE_FILL_PROBE &&
            (address < fixture->uncached_start || address >= fixture->uncached_limit)};
    return LIB_STATUS_OK;
}

static lib_bool cpu_interrupt_memory_timing(void)
{
    static const lib_u8 opcodes[] = {0xccu, 0xcdu, 0xceu};
    static const lib_u8 penalties[] = {2u, 6u, 6u, 17u, 17u, 17u};
    static const lib_u8 bases[] = {26u, 44u, 44u, 71u, 71u, 82u};
    /* Independent four-slot/two-clock write schedule. Real IVT bus reads
     * drain three stack writes; nine VM86 pushes exceed an empty FIFO. Hot
     * entry can also inherit the prior entry's unserviced write tail. */
    static const lib_u8 cold_stall[] = {3u,0u,0u,0u,0u,2u};
    static const struct {
        lib_u32 uncached_start, uncached_limit;
        lib_u8 warm_penalty[3]; /* Real, same-level protected, inner protected. */
        lib_bool repeated_tss_miss;
        lib_u8 warm_stall[6];
    } policies[] = {
        {0u, 0u, {0u, 0u, 0u}, LIB_FALSE, {0u,0u,0u,3u,3u,9u}},
        {0x400u, 0x600u, {2u, 3u, 3u}, LIB_FALSE, {3u,3u,3u,5u,5u,9u}},
        {0x200u, 0x210u, {0u, 3u, 3u}, LIB_FALSE, {0u,3u,3u,5u,5u,9u}},
        {0x600u, 0x610u, {0u, 0u, 13u}, LIB_TRUE, {0u,0u,0u,5u,5u,9u}},
        {0u, LIB_UINT32_MAX, {2u, 6u, 22u}, LIB_TRUE, {3u,0u,0u,0u,0u,2u}}
    };
    const lib_u16 real_vector[] = {0x300u, 0u};

    for (lib_u8 external = 0u; external < 2u; ++external)
    for (lib_u8 mode = 0u; mode < sizeof(penalties); ++mode)
    for (lib_size form = 0u; form < (external ? 1u : sizeof(opcodes)); ++form)
    for (lib_size policy = 0u; policy < sizeof(policies) / sizeof(policies[0]); ++policy) {
        cpu_event_timing_fixture fixture = {0};
        cpu_instruction_fixture *instruction = &fixture.instruction;
        const lib_u8 vector = external ? 0x30u : form == 0u ? 3u : form == 1u ? 0x30u : 4u;
        const lib_u8 code[] = {external ? 0x90u : opcodes[form], vector};
        const lib_u8 bytes = !external && form == 1u ? 2u : 1u;
        const lib_bool gate32 = mode == 2u || mode >= 4u;
        x86_core_cpu_bus_provider bus = cpu_instruction_bus;
        t_cpu before;

        bus.query_cache_memory = cpu_entry_cache_query;
        bus.interrupt_pending = cpu_event_timing_pending;
        bus.acknowledge_interrupt = cpu_event_timing_acknowledge;
        cpu_instruction_prepare_with_bus(instruction, X86_CORE_CPU_PROFILE_80486SX, &bus, &fixture);
        if (mode != 0u) {
            cpu_instruction_prepare_protected_exception(instruction, mode >= 3u ? 4u : 0u);
            if (mode != 5u) instruction->cpu.data.eflags &= ~VCPU_EFLAGS_VM;
            instruction->cpu.data.eflags |= VCPU_EFLAGS_IOPL;
            instruction->cpu.data.idtr.limit = 0x187u;
            lib_memory_copy(instruction->memory + 0x400u + vector * 8u,
                instruction->memory + 0x468u, 8u);
            instruction->memory[0x405u + vector * 8u] = gate32 ? 0xeeu : 0xe6u;
        } else {
            instruction->cpu.data.esp = 0x8000u;
            instruction->cpu.data.idtr.base = 0x400u;
            lib_memory_copy(instruction->memory + 0x400u + vector * 4u, real_vector, sizeof(real_vector));
        }
        fixture.uncached_start = policies[policy].uncached_start;
        fixture.uncached_limit = policies[policy].uncached_limit;
        instruction->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        instruction->cpu.data.eflags |= VCPU_EFLAGS_OF | VCPU_EFLAGS_IF;
        before = instruction->cpu;
        for (lib_u8 warm = 0u; warm < 2u; ++warm) {
            lib_u64 ticks = LIB_UINT64_MAX, nominal = LIB_UINT64_MAX;
            lib_bool modeled = LIB_FALSE;
            const lib_u64 expected = warm ?
                policies[policy].warm_penalty[mode == 0u ? 0u : mode < 3u ? 1u : 2u] :
                penalties[mode] + (mode >= 3u && policies[policy].repeated_tss_miss ? 5u : 0u);
            const lib_u64 setup = form == 1u && (mode == 0u || mode == 5u) ? 4u :
                form == 2u ? 2u : 0u;
            const lib_u64 expected_ticks = external ? bases[mode] + 11u + expected +
                (warm ? policies[policy].warm_stall[mode] : cold_stall[mode]) : expected;
            t_cpu after;

            instruction->cpu = before;
            fixture.vector = vector;
            fixture.pending = external != 0u;
            if (cpu_instruction_run(instruction, code, bytes, &after) != LIB_STATUS_OK ||
                instruction->fault.valid || instruction->delivered_exception.valid ||
                (!external && (!x86_core_cpu_80486_memory_penalty(&instruction->execution, &ticks, &modeled) ||
                    !x86_core_cpu_80486_nominal_instruction_cost(&instruction->execution, &nominal))))
                return LIB_FALSE;
            if (external) {
                ticks = instruction->execution.timing_result.event_ticks;
                nominal = instruction->execution.timing_result.ticks;
            }
            /* The NOP subtotal is already selected before acknowledgement.
             * Entry must preserve it, including fetch and a prior write drain;
             * independent nominal/bus vectors qualify those component costs. */
            if (ticks != expected_ticks || nominal != (external ?
                    fixture.interrupted_ticks : bases[mode] + setup) || nominal == 0u ||
                (!external && (modeled != (expected != 0u && expected != penalties[mode]) ||
                    instruction->execution.timing_result.event_ticks != 0u)) ||
                (external && (fixture.pending || fixture.acknowledgements != warm + 1u)) ||
                after.data.eip != 0x300u) {
                lib_c_printf("INT memory: external=%u mode=%u form=%zu warm=%u policy=%zu miss=%u/%u tss=%u ticks=%llu/%llu nominal=%llu modeled=%u fault=%x\n",
                    (lib_u32)external, (lib_u32)mode, form, (lib_u32)warm, policy,
                    instruction->instructions.data.timing_data_misses,
                    instruction->instructions.data.timing_descriptor_misses,
                    instruction->instructions.data.timing_tss_field_misses,
                    ticks, expected_ticks, nominal, (lib_u32)modeled, instruction->fault.exception_mask);
                return LIB_FALSE;
            }
        }
        x86_core_cpu_execution_finalize(&instruction->execution);
    }
    return LIB_TRUE;
}

static lib_status cpu_event_paging_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    cpu_event_timing_fixture *fixture = opaque;
    if (fixture->reject_address != 0u && fixture->reject_write &&
        address <= fixture->reject_address &&
        (lib_u64)address + bytes > fixture->reject_address) return LIB_STATUS_IO_ERROR;
    if (provenance == X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE) ++fixture->table_writes;
    return cpu_instruction_write(&fixture->instruction, address, source, bytes, provenance, cache_control);
}

static lib_u64 cpu_event_paging_cycle(void *opaque, x86_core_cpu_external_cycle_phase phase,
    x86_core_cpu_external_cycle_space space, lib_u32 address, lib_u8 bytes,
    lib_u8 write, x86_core_cpu_memory_access_provenance provenance)
{
    cpu_event_timing_fixture *fixture = opaque;
    if (phase != X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT ||
        space != X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY) return 0u;
    const lib_u64 transfers = ((address & 3u) + bytes + 3u) / 4u;
    const lib_u64 ticks = write ? 2u * transfers : 1u + transfers;

    fixture->accepted_ticks += ticks;
    if (provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH ||
        provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH) fixture->fetch_ticks += ticks;
    return fixture->wait_ticks;
}

static lib_bool cpu_event_wait_ownership(void)
{
    const lib_u64 waits[] = {0u,1u,9u};
    const lib_u16 vector[] = {0x100u,0u};

    for (lib_size index = 0u; index < sizeof(waits) / sizeof(waits[0]); ++index)
    for (lib_u8 route = 0u; route < 3u; ++route) {
        cpu_event_timing_fixture fixture = {0};
        cpu_instruction_fixture *instruction = &fixture.instruction;
        x86_core_cpu_execution_context *context = &instruction->execution;
        x86_core_cpu_bus_provider bus = cpu_instruction_bus;
        const lib_bool faulting = route == 2u, rejected = route == 1u;
        const lib_u64 expected = faulting ? 33u + 5u * waits[index] :
            rejected ? 7u + 2u * waits[index] : 31u + 4u * waits[index];

        bus.read_memory = cpu_event_descriptor_read;
        bus.write_memory = cpu_event_paging_write;
        cpu_instruction_prepare_with_bus(instruction, X86_CORE_CPU_PROFILE_80486SX, &bus, &fixture);
        instruction->cpu.data.cr0 |= VCPU_CR0_CD | VCPU_CR0_NW;
        instruction->cpu.data.esp = 0x8000u;
        instruction->memory[0] = faulting ? 0x0fu : 0x90u;
        instruction->memory[1] = 0x0bu;
        lib_memory_copy(instruction->memory + (faulting ? 24u : 8u), vector, sizeof(vector));
        fixture.wait_ticks = waits[index];
        fixture.reject_address = rejected ? 0x7ffau : 0u;
        fixture.reject_write = rejected;
        x86_core_cpu_execution_context_bind_external_cycle_provider(context, cpu_event_paging_cycle, &fixture);
        if (!faulting && !x86_core_cpu_request_nmi(context)) return LIB_FALSE;
        x86_core_cpu_execution_refresh(context);
        if (context->timing_result.ticks != (faulting ? 0u : 5u + waits[index]) ||
            context->timing_result.event_ticks != expected || instruction->fault.valid != rejected ||
            context->stop_requested != rejected) {
            lib_c_printf("Event wait ownership: route=%u wait=%llu ticks=%llu event=%llu/%llu fault=%x\n",
                (lib_u32)route, waits[index], context->timing_result.ticks,
                context->timing_result.event_ticks, expected, instruction->fault.exception_mask);
            return LIB_FALSE;
        }
        x86_core_cpu_execution_finalize(context);
    }
    return LIB_TRUE;
}

static lib_bool cpu_interrupt_paging_timing(void)
{
    static const struct {lib_bool write; lib_u32 address;} failures[] = {
        /* Success; PDE/PTE service; A/D service; Gate/descriptor/frame service. */
        {LIB_FALSE, 0u}, {LIB_FALSE, 0x10000u}, {LIB_FALSE, 0x11008u},
        {LIB_TRUE, 0x10000u}, {LIB_TRUE, 0x11008u},
        {LIB_FALSE, 0x2180u}, {LIB_FALSE, 0x3008u}, {LIB_TRUE, 0u},
        /* Guest faults: absent IDT/GDT pages, then a read-only target stack. */
        {LIB_FALSE, 0u}, {LIB_FALSE, 0u}, {LIB_FALSE, 0u}
    };

    for (lib_u8 mode = 0u; mode < 5u; ++mode)
    for (lib_u8 route = 0u; route < 4u; ++route)
    for (lib_u8 updates = 0u; updates < 3u; ++updates)
    for (lib_size failure = 0u; failure < sizeof(failures) / sizeof(failures[0]); ++failure) {
        cpu_event_timing_fixture fixture = {0};
        cpu_instruction_fixture *instruction = &fixture.instruction;
        x86_core_cpu_execution_context *context = &instruction->execution;
        x86_core_cpu_bus_provider bus = cpu_instruction_bus;
        const lib_bool inner = mode >= 2u, vm86 = mode == 4u;
        const lib_bool gate32 = mode == 1u || mode >= 3u;
        const lib_u8 vector = route == 2u ? 2u : 0x30u;
        const lib_u8 code[] = {route ? 0x90u : 0xcdu, vector};
        const lib_u32 walks = inner ? 4u : 3u;
        const lib_u32 stack = inner ? 0x9000u : 0x8000u;
        const lib_u64 nominal = vm86 ? 82u : inner ? 71u : 44u;
        const lib_u64 setup = route == 1u ? 11u : route == 2u ? 3u : route == 3u ? 24u : 0u;
        lib_u32 entry;
        t_cpu before, after;

        if ((route == 3u && failure != 0u) ||
            (failure == 3u && updates != 2u) || (failure == 4u && updates == 0u)) continue;
        bus.read_memory = cpu_event_descriptor_read;
        bus.write_memory = cpu_event_paging_write;
        bus.query_cache_memory = cpu_entry_cache_query;
        bus.interrupt_pending = cpu_event_timing_pending;
        bus.acknowledge_interrupt = cpu_event_timing_acknowledge;
        cpu_instruction_prepare_with_bus(instruction, X86_CORE_CPU_PROFILE_80486SX, &bus, &fixture);
        cpu_instruction_prepare_protected_exception(instruction, inner ? 4u : 0u);
        if (!vm86) instruction->cpu.data.eflags &= ~VCPU_EFLAGS_VM;
        instruction->cpu.data.eflags |= VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL;
        instruction->cpu.data.esp = stack;
        lib_memory_copy(instruction->memory + 0x3000u, instruction->memory + 0x200u, 24u);
        if (inner && !vm86) {
            /* A real outer code/stack pair lets IRET release NMI service and
             * return before checking the same entry with hot translations. */
            lib_memory_copy(instruction->memory + 0x3018u,
                (const lib_u8[]){0xffu, 0xffu, 0u, 0u, 0u, 0xfbu, 0u, 0u,
                    0xffu, 0xffu, 0u, 0u, 0u, 0xf3u, 0u, 0u}, 16u);
            instruction->cpu.data.cs.selector = 0x1bu;
            instruction->cpu.data.ss.selector = 0x23u;
            instruction->cpu.data.gdtr.limit = 39u;
        }
        lib_memory_copy(instruction->memory + 0x4000u, instruction->memory + 0x600u, 0x68u);
        lib_memory_copy(instruction->memory + 0x4004u, &stack, sizeof(stack));
        lib_memory_copy(instruction->memory + 0x2000u + vector * 8u,
            instruction->memory + 0x468u, 8u);
        instruction->memory[0x2005u + vector * 8u] = gate32 ? 0xeeu : 0xe6u;
        for (lib_u8 exception = 8u; exception <= 14u; exception += 6u)
            lib_memory_copy(instruction->memory + 0x2000u + exception * 8u,
                instruction->memory + 0x2000u + vector * 8u, 8u);
        instruction->cpu.data.idtr.base = 0x2000u;
        instruction->cpu.data.idtr.limit = 0x187u;
        instruction->cpu.data.gdtr.base = 0x3000u;
        instruction->cpu.data.tr.base = 0x4000u;
        fixture.uncached_start = 0x10000u;
        fixture.uncached_limit = 0x12000u;
        instruction->cpu.data.cr0 |= VCPU_CR0_PG;
        instruction->cpu.data.cr3 = 0x10000u;
        entry = 0x11027u;
        lib_memory_copy(instruction->memory + 0x10000u, &entry, sizeof(entry));
        for (lib_u32 page = 0u; page < 16u; ++page) {
            entry = (page << 12u) | 0x67u;
            lib_memory_copy(instruction->memory + 0x11000u + page * 4u, &entry, sizeof(entry));
        }
        /* Warm only the interrupted code translation before independently
         * selecting the entry pages' A/D state. No fake TLB entry is installed. */
        instruction->memory[0x100u] = 0x90u;
        instruction->cpu.data.eip = 0x100u;
        x86_core_cpu_execution_refresh(context);
        if (instruction->fault.valid || context->stop_requested) return LIB_FALSE;
        instruction->cpu.data.eip = 0u;
        x86_core_cpu_execution_invalidate_prefetch(context);
        entry = updates == 2u ? 0x11007u : 0x11027u;
        lib_memory_copy(instruction->memory + 0x10000u, &entry, sizeof(entry));
        for (lib_u32 page = 2u; page <= 8u; ++page) {
            entry = (page << 12u) | (updates ? 7u : 0x67u);
            lib_memory_copy(instruction->memory + 0x11000u + page * 4u, &entry, sizeof(entry));
        }
        if (failure >= 8u) {
            const lib_u32 page = failure == 8u ? 2u : failure == 9u ? 3u : (stack - 1u) >> 12u;
            entry = page << 12u;
            if (failure == 10u) entry |= updates ? 5u : 0x65u;
            lib_memory_copy(instruction->memory + 0x11000u + page * 4u, &entry, sizeof(entry));
            instruction->cpu.data.cr0 |= VCPU_CR0_WP;
        }
        if (route == 3u) {
            const x86_core_cpu_debug_register_patch patch = {
                .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR3),
                .values = {[X86_CORE_CPU_DEBUG_CR3] = 0x10000u}
            };
            /* Use the existing serialized patch boundary to invalidate the
             * warmed code translation, then fault its real next fetch. */
            if (x86_core_cpu_debug_patch_registers(context, &patch) != LIB_STATUS_OK) return LIB_FALSE;
            entry = 0u;
            lib_memory_copy(instruction->memory + 0x11000u, &entry, sizeof(entry));
        }
        before = instruction->cpu;
        fixture.reject_address = failure == 7u ? stack - (gate32 ? 4u : 2u) :
            failures[failure].address;
        if (failure == 5u) fixture.reject_address = 0x2000u + vector * 8u;
        fixture.reject_write = failures[failure].write;
        x86_core_cpu_execution_context_bind_external_cycle_provider(context, cpu_event_paging_cycle, &fixture);
        for (lib_u8 warm = 0u; warm < (failure || route == 3u ? 1u : 2u); ++warm) {
            const lib_u64 translation = warm ? 0u : updates == 0u ? walks * 13u :
                updates == 1u ? walks * 21u : 28u + (walks - 1u) * 21u;
            const lib_u64 memory = warm ? 0u : inner ? 17u : 6u;
            /* IRET drains the NMI frame before its hot repeat. A directly
             * repeated INTR instead retains the previous five/nine writes. */
            const lib_u64 stall = vm86 ? (warm && route != 2u ? 9u : 2u) :
                inner && warm && route != 2u ? 3u : 0u;
            const lib_u32 writes = warm || updates == 0u ? 0u : walks + (updates == 2u);
            const lib_u32 reads = warm ? 0u : 2u * walks + writes;
            lib_u64 ticks = LIB_UINT64_MAX;
            lib_bool modeled = LIB_TRUE;

            instruction->cpu = before;
            fixture.accepted_ticks = fixture.fetch_ticks = 0u;
            fixture.table_reads = fixture.table_writes = 0u;
            fixture.vector = vector;
            fixture.pending = route == 1u;
            if (route == 2u && !x86_core_cpu_request_nmi(context)) return LIB_FALSE;
            const lib_status status = cpu_instruction_run(instruction, code, route ? 1u : 2u, &after);
            if (route == 3u) {
                const lib_u64 prefix = updates == 2u ? 8u : 4u;
                const lib_u64 entry_translation = updates == 2u ? walks * 21u : translation;
                /* The failed fetch accepts two table reads. If PDE.A was clear,
                 * its locked read/write also commits before the absent PTE is
                 * found: the following PF entry then sees an accessed PDE. */
                if (status != LIB_STATUS_OK || instruction->fault.valid ||
                    !instruction->delivered_exception.valid ||
                    instruction->delivered_exception.exception_mask != VCPUINS_EXCEPT_PF ||
                    after.data.eip != 0x300u || context->timing_result.ticks != 0u ||
                    context->timing_result.event_ticks != prefix + nominal + setup + entry_translation + memory +
                        (vm86 ? 3u : 0u) ||
                    fixture.table_reads != reads + 2u || fixture.table_writes != writes) {
                    lib_c_printf("Paged PF entry mode=%u updates=%u event=%llu/%llu reads=%u/%u fault=%x delivered=%x\n",
                        (lib_u32)mode, (lib_u32)updates, context->timing_result.event_ticks,
                        prefix + nominal + setup + entry_translation + memory,
                        fixture.table_reads, reads + 2u, instruction->fault.exception_mask,
                        instruction->delivered_exception.exception_mask);
                    return LIB_FALSE;
                }
            } else if (failure >= 8u) {
                const lib_u64 accepted = fixture.accepted_ticks - (route ? fixture.fetch_ticks : 0u);
                const lib_u64 indication = route ? 0u : 2u;
                if (status != LIB_STATUS_OK || instruction->fault.valid ||
                    !x86_core_cpu_is_shutdown(context) || !instruction->delivered_exception.valid ||
                    instruction->delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN ||
                    context->timing_result.event_ticks != accepted + setup + 24u + indication ||
                    instruction->instructions.data.memory_lock_active) {
                    lib_c_printf("Paged nested failure mode=%u route=%u updates=%u case=%zu event=%llu/%llu delivered=%x fault=%x\n",
                        (lib_u32)mode, (lib_u32)route, (lib_u32)updates, failure,
                        context->timing_result.event_ticks, accepted + setup + 24u + indication,
                        instruction->delivered_exception.exception_mask, instruction->fault.exception_mask);
                    return LIB_FALSE;
                }
                /* Async entry occurs after this round's special-cycle poll;
                 * its shutdown indication is therefore the next CPU round. */
                x86_core_cpu_execution_refresh(context);
                if (context->timing_result.ticks ||
                    context->timing_result.event_ticks != (route ? 2u : 0u)) return LIB_FALSE;
                x86_core_cpu_execution_refresh(context);
                if (context->timing_result.ticks || context->timing_result.event_ticks) return LIB_FALSE;
            } else if (failure) {
                const lib_u64 accepted = fixture.accepted_ticks - (route ? fixture.fetch_ticks : 0u);
                if (status != LIB_STATUS_INTERNAL_ERROR || !instruction->fault.valid ||
                    instruction->fault.exception_mask != VCPUINS_EXCEPT_CE ||
                    /* Code translation is warm, but this first cache line is
                     * not: the retired NOP owns its five-clock physical fill. */
                    context->timing_result.ticks != (route ? 5u : 0u) ||
                    context->timing_result.event_ticks != accepted + setup ||
                    instruction->instructions.data.memory_lock_active) {
                    lib_c_printf("Paged entry failure mode=%u route=%u updates=%u case=%zu event=%llu/%llu ticks=%llu fault=%x\n",
                        (lib_u32)mode, (lib_u32)route, (lib_u32)updates, failure,
                        context->timing_result.event_ticks, accepted + setup,
                        context->timing_result.ticks, instruction->fault.exception_mask);
                    return LIB_FALSE;
                }
            } else if (status != LIB_STATUS_OK || instruction->fault.valid ||
                instruction->delivered_exception.valid || after.data.eip != 0x300u ||
                fixture.table_reads != reads || fixture.table_writes != writes ||
                (route ? context->timing_result.event_ticks != nominal + setup + translation + memory + stall :
                    !x86_core_cpu_80486_memory_penalty(context, &ticks, &modeled) ||
                    ticks != translation + memory)) {
                lib_c_printf("Paged entry mode=%u route=%u updates=%u warm=%u read=%u/%u write=%u/%u ticks=%llu event=%llu expected=%llu fault=%x\n",
                    (lib_u32)mode, (lib_u32)route, (lib_u32)updates, (lib_u32)warm,
                    fixture.table_reads, reads, fixture.table_writes, writes, ticks,
                    context->timing_result.event_ticks, translation + memory, instruction->fault.exception_mask);
                return LIB_FALSE;
            }
            if (!failure && route == 2u && warm == 0u) {
                instruction->memory[0x300u] = gate32 ? 0x66u : 0xcfu;
                instruction->memory[0x301u] = 0xcfu;
                x86_core_cpu_execution_invalidate_prefetch(context);
                x86_core_cpu_execution_refresh(context);
                if (instruction->fault.valid || context->nmi_in_service) return LIB_FALSE;
            }
        }
        x86_core_cpu_execution_finalize(context);
    }
    return LIB_TRUE;
}

static lib_bool cpu_vm86_exception_timing(void)
{
    static const struct {
        lib_u8 code[2];
        lib_u8 clocks;
        lib_u8 prefix_ticks;
    } cases[] = {
        {{0xfau, 0x90u}, 90u, 5u}, {{0xfbu, 0x90u}, 90u, 5u},
        {{0x9cu, 0x90u}, 91u, 5u}, {{0x9du, 0x90u}, 90u, 5u},
        {{0xcfu, 0x90u}, 91u, 5u}, {{0xcdu, 0x20u}, 91u, 5u},
        {{0xe4u, 0x80u}, 132u, 7u}, {{0xe5u, 0x80u}, 132u, 7u},
        {{0xe6u, 0x80u}, 132u, 7u}, {{0xe7u, 0x80u}, 132u, 7u},
        {{0xecu, 0x90u}, 133u, 7u}, {{0xedu, 0x90u}, 133u, 7u},
        {{0xeeu, 0x90u}, 133u, 7u}, {{0xefu, 0x90u}, 133u, 7u},
        {{0x6cu, 0x90u}, 132u, 7u}, {{0x6du, 0x90u}, 132u, 7u},
        {{0x6eu, 0x90u}, 132u, 9u}, {{0x6fu, 0x90u}, 132u, 9u},
        {{0xf3u, 0x6cu}, 133u, 7u}, {{0xf3u, 0x6du}, 133u, 7u},
        {{0xf3u, 0x6eu}, 133u, 9u}, {{0xf3u, 0x6fu}, 133u, 9u}
    };
    const lib_u16 absent_bitmap = 0xffffu;

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        cpu_instruction_fixture fixture;
        x86_core_cpu_timing_result timing;
        /* Non-filling IDT/CS/SS plus two TSS reads; ten GP-frame writes
         * contribute three actual FIFO-full stalls in the L2 bus schedule. */
        const lib_u32 entry_miss_ticks = 22u + 3u;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        cpu_instruction_prepare_protected_exception(&fixture, 4u);
        fixture.cpu.data.ecx = 2u;
        fixture.cpu.data.edx = 0x80u;
        lib_memory_copy(fixture.memory + 0x666u, &absent_bitmap, sizeof(absent_bitmap));
        if (cpu_instruction_run(&fixture, cases[index].code, sizeof(cases[index].code),
                &after) != LIB_STATUS_OK || fixture.fault.valid ||
            !fixture.delivered_exception.valid ||
            fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_GP)
            return LIB_FALSE;
        timing = x86_core_cpu_capture_timing(&fixture.execution);
        if (timing.ticks != 0u ||
            timing.event_ticks != (lib_u64)cases[index].clocks + cases[index].prefix_ticks + entry_miss_ticks) {
            lib_c_printf("VM86 exception timing: case=%zu expected=%u actual=%llu\n",
                index, (lib_u32)cases[index].clocks + cases[index].prefix_ticks + entry_miss_ticks,
                timing.event_ticks);
            return LIB_FALSE;
        }
        x86_core_cpu_execution_finalize(&fixture.execution);
    }
    return LIB_TRUE;
}

static lib_bool cpu_debug_trap_before_fetch(void)
{
    static const struct {lib_u8 code[2]; lib_u8 bytes;} cases[] = {
        {{0x90u,0u},1u}, {{0x66u,0x90u},2u}, {{0x67u,0x90u},2u}
    };
    const lib_u16 vector[] = {0x100u,0u};

    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        lib_u16 frame[3] = {0};

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cs.limit = cases[index].bytes - 1u;
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.esp = 0x8000u;
        fixture.cpu.data.eflags = 2u | VCPU_EFLAGS_TF | VCPU_EFLAGS_IF;
        lib_memory_copy(fixture.memory + 4u, vector, sizeof(vector));
        if (cpu_instruction_run(&fixture, cases[index].code, cases[index].bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || !fixture.delivered_exception.valid ||
            fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_DB ||
            after.data.eip != 0x100u || after.data.esp != 0x7ffau ||
            fixture.execution.timing_result.ticks == 0u ||
            fixture.execution.timing_result.event_ticks != 28u) {
            lib_c_printf("Debug trap/fetch priority: code32=%u form=%zu eip=%x mask=%x event=%llu\n",
                (lib_u32)code32, index, after.data.eip,
                fixture.delivered_exception.exception_mask, fixture.execution.timing_result.event_ticks);
            return LIB_FALSE;
        }
        lib_memory_copy(frame, fixture.memory + 0x7ffau, sizeof(frame));
        if (frame[0] != cases[index].bytes || frame[1] != 0u ||
            frame[2] != (2u | VCPU_EFLAGS_TF | VCPU_EFLAGS_IF) || fixture.writes != 3u)
            return LIB_FALSE;
        x86_core_cpu_execution_finalize(&fixture.execution);
    }
    return LIB_TRUE;
}

static lib_bool cpu_event_entry_timing(void)
{
    const lib_u16 real_vector[] = {0x100u, 0u};
    static const lib_u8 vectors[] = {6u, 2u, 0x30u, 2u, 0x30u, 2u, 0x30u};
    static const lib_u8 entry_clocks[] = {26u, 44u, 44u, 82u, 71u};
    static const lib_u8 entry_miss_clocks[] = {2u, 6u, 6u, 22u, 22u};

    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_80386;
         profile <= X86_CORE_CPU_PROFILE_80486SX; ++profile)
    for (lib_u8 mode = 0u; mode < sizeof(entry_clocks); ++mode)
    for (lib_u8 route = 0u; route < sizeof(vectors); ++route) {
        cpu_event_timing_fixture fixture = {0};
        x86_core_cpu_bus_provider bus = cpu_instruction_bus;
        cpu_instruction_fixture *instruction = &fixture.instruction;
        const lib_bool nmi = route == 1u || route == 3u;
        const lib_bool halted = route == 3u || route == 4u;
        const lib_bool reject = route == 6u;
        const lib_bool retired = route != 0u && !halted;
        const lib_u64 expected = profile != X86_CORE_CPU_PROFILE_80486SX || reject ?
            0u : entry_clocks[mode] + entry_miss_clocks[mode] + (mode == 3u ? 2u : 0u) +
                (route == 0u ? 5u : nmi ? 3u : 11u);
        x86_core_cpu_timing_result timing;
        x86_core_cpu_instruction_observation completed;

        bus.interrupt_pending = cpu_event_timing_pending;
        bus.acknowledge_interrupt = cpu_event_timing_acknowledge;
        cpu_instruction_prepare_with_bus(instruction, profile, &bus, &fixture);
        if (mode != 0u) {
            cpu_instruction_prepare_protected_exception(instruction,
                mode == 1u ? 0u : mode == 2u ? 3u : 4u);
            if (mode == 4u) instruction->cpu.data.eflags &= ~VCPU_EFLAGS_VM;
            instruction->cpu.data.idtr.limit = 0x187u;
            lib_memory_copy(instruction->memory + 0x400u + vectors[route] * 8u,
                instruction->memory + 0x468u, 8u);
        } else {
            lib_memory_copy(instruction->memory + vectors[route] * 4u,
                real_vector, sizeof(real_vector));
        }
        instruction->memory[0] = route == 0u ? 0x0fu : 0x90u;
        instruction->memory[1] = route == 0u ? 0x0bu : 0x90u;
        instruction->memory[mode ? 0x300u : 0x100u] = 0x90u;
        instruction->cpu.data.eflags |= VCPU_EFLAGS_IF;
        instruction->cpu.data.flagHalt = halted;
        fixture.pending = route != 0u && !nmi;
        fixture.reject_acknowledge = reject;
        fixture.vector = vectors[route];
        if (nmi && !x86_core_cpu_request_nmi(&instruction->execution)) return LIB_FALSE;
        x86_core_cpu_execution_refresh(&instruction->execution);
        timing = x86_core_cpu_capture_timing(&instruction->execution);
        if (timing.event_ticks != expected || (timing.ticks != 0u) != retired ||
            instruction->execution.stop_requested != reject || instruction->fault.valid != reject ||
            fixture.acknowledgements != (route != 0u && !nmi ? 1u : 0u) ||
            (!reject && (instruction->cpu.data.eip != (mode ? 0x300u : 0x100u) ||
                x86_core_cpu_execution_consume_instruction_fault_delivery(
                    &instruction->execution) != !retired))) {
            lib_c_printf("Entry timing: profile=%u mode=%u route=%u ticks=%llu event=%llu expected=%llu\n",
                (lib_u32)profile, mode, route, timing.ticks, timing.event_ticks, expected);
            return LIB_FALSE;
        }
        if (retired) {
            x86_core_cpu_execution_copy_observation(&instruction->execution, &completed);
            if (completed.old_eip != 0u || completed.point.eip != 1u ||
                completed.point.bytes[0] != 0x90u || completed.cpl != (mode < 2u ? 0u : 3u) ||
                completed.protected_mode != (mode != 0u) ||
                completed.virtual_8086_mode != (mode == 3u)) {
                lib_c_printf("Completed entry identity: profile=%u mode=%u route=%u old=%x next=%x cpl=%u vm=%u\n",
                    (lib_u32)profile, mode, route, completed.old_eip, completed.point.eip,
                    (lib_u32)completed.cpl, (lib_u32)completed.virtual_8086_mode);
                return LIB_FALSE;
            }
        }
        if (!reject) {
            x86_core_cpu_execution_refresh(&instruction->execution);
            timing = x86_core_cpu_capture_timing(&instruction->execution);
            if (timing.event_ticks != 0u || timing.ticks == 0u ||
                instruction->execution.stop_requested || instruction->fault.valid)
                return LIB_FALSE;
        }
        x86_core_cpu_execution_finalize(&instruction->execution);
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    lib_bool failed = LIB_FALSE;
    failed |= !cpu_debug_trap_before_fetch();
    failed |= !cpu_event_descriptor_once();
    failed |= !cpu_interrupt_memory_timing();
    failed |= !cpu_interrupt_paging_timing();
    failed |= !cpu_event_wait_ownership();
    failed |= !cpu_entry_failed_prefix_timing();
    failed |= !cpu_event_entry_timing();
    failed |= !cpu_vm86_exception_timing();
    failed |= !cpu_sx_reserved_escaped_slots();
    failed |= !cpu_sx_reserved_modrm_forms();
    failed |= !cpu_real_entry_new_cs();
    failed |= !cpu_early_fixed_ivt();
    failed |= !cpu_shutdown_protected_recovery();
    failed |= !cpu_shutdown_recovery();
    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_80386;
         profile <= X86_CORE_CPU_PROFILE_80486SX; ++profile) {
        failed |= !cpu_entry_provider_failure(profile);
        failed |= !cpu_external_error_matrix(profile);
        failed |= !cpu_string_page_restart(profile);
    }
    failed |= !cpu_exception_pair_matrix();
    failed |= !cpu_286_double_fault_task(LIB_FALSE);
    failed |= !cpu_286_double_fault_task(LIB_TRUE);
    for (lib_u8 scenario = 0u; scenario < 4u; ++scenario) {
        failed |= !cpu_page_delivery_case(X86_CORE_CPU_PROFILE_80386, scenario);
        failed |= !cpu_page_delivery_case(X86_CORE_CPU_PROFILE_80486SX, scenario);
    }
    failed |= !cpu_divide_return_matrix();
    failed |= !cpu_aam_return_matrix();
    if (!cpu_compare_fault_flags()) {
        lib_c_printf("COMPARE-FAULT-FLAGS failed\n"); failed = LIB_TRUE;
    }
    if (!cpu_compare_flags_lifetime()) {
        lib_c_printf("COMPARE-FLAGS-LIFETIME failed\n"); failed = LIB_TRUE;
    }
    if (failed) return 1;
    if (cpu_ud_cache_preservation() || cpu_interrupt_pending_and_rollback()) return 1;
    lib_c_printf("%s\n", "CPU-EXECUTION-FAULT-EVENT:OK");
    return 0;
}
