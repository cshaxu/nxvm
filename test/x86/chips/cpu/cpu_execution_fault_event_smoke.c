#include "support/cpu_bus_fixture.h"
#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* Private pending-event and whole-cache invariants belong to the CPU owner;
 * the board interrupt corpus separately checks PIC, frames and transactions. */
static void cpu_interrupt_prepare(cpu_bus_fixture *fixture, lib_u8 vector,
    lib_u8 gate_type)
{
    const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0xfau,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0
    };
    const lib_u8 gate[] = {0u,1u,0x0bu,0u,0u,gate_type,0u,0u};
    t_cpu *cpu = &fixture->cpu;

    cpu_bus_prepare(fixture, CORE_MACHINE_CPU_PROFILE_80386);
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

            cpu_interrupt_prepare(&fixture, 6u, reject ? 0x80u : 0x8eu);
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
            core_machine_cpu_execution_refresh(&fixture.execution);
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
                if (!core_machine_cpu_is_shutdown(&fixture.execution) || fixture.faults ||
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

        cpu_interrupt_prepare(&fixture, 2u, reject ? 0x80u : 0x8eu);
        fixture.memory[0] = 0x90u;
        fixture.cpu.data.eflags = 0x202u;
        if (!core_machine_cpu_request_nmi(&fixture.execution)) return 1;
        before = fixture.cpu;
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.nmi_pending != reject) return 1;
        if (reject) {
            if (!core_machine_cpu_is_shutdown(&fixture.execution) || fixture.faults ||
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

        cpu_interrupt_prepare(&fixture, 0x0du, gate);
        lib_memory_copy(fixture.memory, code, sizeof(code));
        if (failure == 2u) fixture.memory[0x30du] = 0x7au;
        if (failure == 3u) fixture.cpu.data.ss.limit = 0x7ffeu;
        before = fixture.cpu;
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (!core_machine_cpu_is_shutdown(&fixture.execution) || fixture.faults ||
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
} cpu_entry_io_fixture;

static lib_status cpu_entry_io_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch)
{
    cpu_entry_io_fixture *fixture = (cpu_entry_io_fixture *)opaque;
    if (!fixture->reject_write && address == fixture->reject_address)
        return LIB_STATUS_IO_ERROR;
    return cpu_bus_read(&fixture->state, address, destination, bytes,
        provenance, observe_only, reset_fetch);
}

static lib_status cpu_entry_io_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance)
{
    cpu_entry_io_fixture *fixture = (cpu_entry_io_fixture *)opaque;
    if (fixture->reject_write && address == fixture->reject_address)
        return LIB_STATUS_IO_ERROR;
    return cpu_bus_write(&fixture->state, address, source, bytes, provenance);
}

static lib_bool cpu_entry_provider_failure(void)
{
    const lib_u8 code[] = {0x0fu,0x01u,0xf0u};
    const lib_u8 df_gate[] = {0u,1u,0x0bu,0u,0u,0x8eu,0u,0u};
    lib_u8 double_fault, write;

    for (double_fault = 0u; double_fault < 2u; ++double_fault)
    for (write = 0u; write < 2u; ++write) {
        cpu_entry_io_fixture fixture;
        core_machine_cpu_bus_provider bus = cpu_bus_provider;
        lib_u32 prior_flags = 0u;
        cpu_interrupt_prepare(&fixture.state, 13u, double_fault ? 0x80u : 0x8eu);
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
        core_machine_cpu_execution_refresh(&fixture.state.execution);
        lib_memory_copy(&prior_flags, fixture.state.memory + 0x7fcu, sizeof(prior_flags));
        if (!fixture.state.execution.stop_requested || fixture.state.faults != 1u ||
            fixture.state.fault.exception_mask != VCPUINS_EXCEPT_CE ||
            fixture.state.fault.exception_code != fixture.reject_address ||
            fixture.state.execution.shutdown_requested ||
            (write && (fixture.state.writes == 0u || prior_flags == 0u))) {
            lib_c_printf("Entry IO df=%u write=%u stop=%u fault=%u/%x/%x shutdown=%u rejected=%x\n",
                double_fault, write, fixture.state.execution.stop_requested,
                fixture.state.faults, fixture.state.fault.exception_mask,
                fixture.state.fault.exception_code, fixture.state.execution.shutdown_requested,
                fixture.reject_address);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool cpu_external_error_matrix(void)
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
        cpu_bus_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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
        if (source == 1u) core_machine_cpu_request_nmi(&state.execution);
        state.interrupt = source == 2u;
        core_machine_cpu_execution_refresh(&state.execution);
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
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const struct { lib_u8 vector; lib_bool first_286; lib_u8 class_386; } rows[] = {
        {0u,LIB_TRUE,1u}, {1u,LIB_FALSE,0u}, {2u,LIB_FALSE,0u},
        {3u,LIB_FALSE,0u}, {4u,LIB_FALSE,0u}, {5u,LIB_FALSE,0u},
        {6u,LIB_FALSE,0u}, {7u,LIB_FALSE,0u}, {9u,LIB_FALSE,1u},
        {10u,LIB_TRUE,1u}, {11u,LIB_TRUE,1u}, {12u,LIB_TRUE,1u},
        {13u,LIB_TRUE,1u}, {14u,LIB_FALSE,2u}, {16u,LIB_FALSE,0u}
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
        if (profiles[profile] == CORE_MACHINE_CPU_PROFILE_80286) {
            if (rows[first].vector == 14u || rows[second].vector == 14u) continue;
            expected = rows[first].first_286;
        } else if (profiles[profile] == CORE_MACHINE_CPU_PROFILE_80386)
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

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);
    core_machine_cpu_execution_load_segment(&state.execution, &state.cpu.data.cs, 0x0200u);
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
    core_machine_cpu_execution_refresh(&state.execution);
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

static lib_bool cpu_page_delivery_case(lib_u8 scenario)
{
    const lib_bool page_first = scenario >= 2u;
    const lib_bool missing_descriptor = scenario != 0u;
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

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    core_machine_cpu_execution_load_segment(&state.execution, &state.cpu.data.cs, 0x0200u);
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
    core_machine_cpu_execution_refresh(&state.execution);
    lib_memory_copy(frame, state.memory + 0x7ff0u, sizeof(frame));
    if (state.execution.stop_requested || state.fault.valid ||
        !state.delivered_exception.valid ||
        state.delivered_exception.exception_mask != (page_first ? VCPUINS_EXCEPT_DF :
            missing_descriptor ? VCPUINS_EXCEPT_PF : VCPUINS_EXCEPT_GP) ||
        state.delivered_exception.exception_code != 0u ||
        state.delivered_exception.point.eip != 0u ||
        state.cpu.data.cr2 != (scenario == 2u ? 0x1000u : missing_descriptor ? 0x1308u : 0u) ||
        state.cpu.data.eip != (page_first ? 0x400u : missing_descriptor ? 0x300u : 0x100u) ||
        state.cpu.data.cs.selector != (page_first ? 8u : missing_descriptor ? 0x1bu : 0x100bu) ||
        state.cpu.data.sp != 0x7ff0u || frame[0] != 0u || frame[1] != 0u ||
        frame[2] != (page_first ? 8u : 0x0bu)) {
        lib_c_printf("Page delivery scenario=%u stop=%u terminal=%x delivered=%x cr2=%x ip=%x sp=%x frame=%x/%x/%x\n",
            scenario, state.execution.stop_requested, state.fault.exception_mask,
            state.delivered_exception.exception_mask, state.cpu.data.cr2,
            state.cpu.data.eip, state.cpu.data.sp, frame[0], frame[1], frame[2]);
        return LIB_FALSE;
    }
    if (!missing_descriptor) lib_c_printf("Serial GP/PF fixture: mapped GP control passed\n");
    return LIB_TRUE;
}

static lib_bool cpu_aam_return_matrix(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
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
        const lib_u16 expected_ip = profiles[profile] < CORE_MACHINE_CPU_PROFILE_80186 ?
            2u + prefix : 0u;
        cpu_instruction_prepare(&state, profiles[profile]);
        core_machine_cpu_execution_load_segment(&state.execution, &state.cpu.data.cs, 0x0200u);
        state.cpu.data.esp = 0x8000u;
        state.cpu.data.eax = 42u;
        state.cpu.data.eflags = 2u;
        lib_memory_copy(state.memory, vector, sizeof(vector));
        lib_memory_copy(state.memory + 0x2000u, code + (prefix ? 0u : 1u), 2u + prefix);
        core_machine_cpu_execution_refresh(&state.execution);
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
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
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
    for (wide = 0u; wide < (profiles[profile] == CORE_MACHINE_CPU_PROFILE_80386 &&
        forms[form].width == 2u ? 2u : 1u); ++wide)
    for (overflow = 0u; overflow < 2u; ++overflow) {
        cpu_instruction_fixture state;
        lib_u8 code[6] = {0};
        lib_u8 length = 0u;
        const lib_u8 width = wide ? 4u : forms[form].width;
        const lib_u32 divisor = overflow ? 1u : 0u;
        lib_u16 frame[3] = {0};
        const lib_u16 expected_ip = profiles[profile] < CORE_MACHINE_CPU_PROFILE_80186 ?
            forms[form].length + (lib_u32)segment + wide : 0u;

        cpu_instruction_prepare(&state, profiles[profile]);
        core_machine_cpu_execution_load_segment(&state.execution,
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
        core_machine_cpu_execution_refresh(&state.execution);
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
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
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
            core_machine_cpu_is_shutdown(&fixture.execution) ||
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
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186
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
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
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
            reject ? 0x80u : (profiles[index] == CORE_MACHINE_CPU_PROFILE_80286 ?
                0x86u : 0x8eu),0u,0u};

        cpu_instruction_prepare(&fixture, profiles[index]);
        lib_memory_copy(fixture.memory + 0x300u, gdt, sizeof(gdt));
        lib_memory_copy(fixture.memory + 0x400u + 2u * 8u, gate, sizeof(gate));
        fixture.cpu.data.gdtr.base = 0x300u;
        fixture.cpu.data.gdtr.limit = sizeof(gdt) - 1u;
        fixture.cpu.data.idtr.base = 0x400u;
        fixture.cpu.data.idtr.limit = 0x17u;
        fixture.cpu.data.cr0 |= VCPU_CR0_PE;
        core_machine_cpu_execution_load_segment(&fixture.execution, &fixture.cpu.data.cs, 8u);
        core_machine_cpu_execution_load_segment(&fixture.execution, &fixture.cpu.data.ss, 0x10u);
        fixture.cpu.data.esp = 0x8000u;
        fixture.cpu.data.eip = 0x20u;
        core_machine_cpu_execution_request_shutdown(&fixture.execution);
        if (!core_machine_cpu_request_nmi(&fixture.execution)) return LIB_FALSE;
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (fixture.fault.valid || fixture.execution.stop_requested ||
            !(fixture.cpu.data.cr0 & VCPU_CR0_PE) ||
            core_machine_cpu_is_shutdown(&fixture.execution) != (reject != 0u))
            return LIB_FALSE;
        if (!reject) {
            if (fixture.cpu.data.eip != 0x100u || fixture.cpu.data.cs.selector != 8u ||
                fixture.cpu.data.esp != 0x8000u -
                    (profiles[index] == CORE_MACHINE_CPU_PROFILE_80286 ? 6u : 12u) ||
                !core_machine_cpu_execution_consume_instruction_fault_delivery(
                    &fixture.execution)) return LIB_FALSE;
        } else {
            fixture.memory[0x415u] = profiles[index] == CORE_MACHINE_CPU_PROFILE_80286 ?
                0x86u : 0x8eu;
            if (!core_machine_cpu_request_nmi(&fixture.execution)) return LIB_FALSE;
            core_machine_cpu_execution_refresh(&fixture.execution);
            if (core_machine_cpu_is_shutdown(&fixture.execution) !=
                    (profiles[index] == CORE_MACHINE_CPU_PROFILE_80286)) return LIB_FALSE;
        }
        core_machine_cpu_state_reset(&fixture.execution);
        if (core_machine_cpu_is_shutdown(&fixture.execution) ||
            (fixture.cpu.data.cr0 & VCPU_CR0_PE)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cpu_shutdown_recovery(void)
{
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
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
        core_machine_cpu_execution_request_shutdown(&fixture.execution);
        if (!core_machine_cpu_execution_consume_shutdown_request(&fixture.execution) ||
            !core_machine_cpu_is_shutdown(&fixture.execution)) return LIB_FALSE;
        fixture.interrupt = LIB_TRUE;
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (!core_machine_cpu_is_shutdown(&fixture.execution) || fixture.acknowledgements ||
            fixture.instruction_count || lib_memory_compare(&before, &fixture.cpu,
                sizeof(before))) return LIB_FALSE;
        if (!core_machine_cpu_request_nmi(&fixture.execution)) return LIB_FALSE;
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (core_machine_cpu_is_shutdown(&fixture.execution) || fixture.faults ||
            fixture.acknowledgements || fixture.cpu.data.eip != 0x0200u ||
            fixture.cpu.data.sp != before.data.sp - 6u ||
            !core_machine_cpu_execution_consume_instruction_fault_delivery(
                &fixture.execution)) return LIB_FALSE;
        /* A failed shutdown-NMI on 286 cannot be retried without RESET. */
        cpu_bus_prepare(&fixture, profiles[index]);
        fixture.cpu.data.idtr.limit = 0u;
        core_machine_cpu_execution_request_shutdown(&fixture.execution);
        if (!core_machine_cpu_request_nmi(&fixture.execution)) return LIB_FALSE;
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (!core_machine_cpu_is_shutdown(&fixture.execution) || fixture.faults ||
            fixture.execution.stop_requested) return LIB_FALSE;
        fixture.cpu.data.idtr.limit = 0x03ffu;
        lib_memory_copy(fixture.memory + 8u, nmi_vector, sizeof(nmi_vector));
        if (!core_machine_cpu_request_nmi(&fixture.execution)) return LIB_FALSE;
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (core_machine_cpu_is_shutdown(&fixture.execution) !=
                (profiles[index] == CORE_MACHINE_CPU_PROFILE_80286))
            return LIB_FALSE;
        core_machine_cpu_execution_request_shutdown(&fixture.execution);
        core_machine_cpu_state_reset(&fixture.execution);
        if (core_machine_cpu_is_shutdown(&fixture.execution) ||
            core_machine_cpu_execution_consume_shutdown_request(&fixture.execution))
            return LIB_FALSE;
        core_machine_cpu_execution_request_shutdown(&fixture.execution);
        core_machine_cpu_state_initialize(&fixture.execution);
        if (core_machine_cpu_is_shutdown(&fixture.execution) ||
            core_machine_cpu_execution_consume_shutdown_request(&fixture.execution))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    lib_bool failed = LIB_FALSE;
    failed |= !cpu_real_entry_new_cs();
    failed |= !cpu_early_fixed_ivt();
    failed |= !cpu_shutdown_protected_recovery();
    failed |= !cpu_shutdown_recovery();
    failed |= !cpu_entry_provider_failure();
    failed |= !cpu_external_error_matrix();
    failed |= !cpu_exception_pair_matrix();
    failed |= !cpu_286_double_fault_task(LIB_FALSE);
    failed |= !cpu_286_double_fault_task(LIB_TRUE);
    for (lib_u8 scenario = 0u; scenario < 4u; ++scenario)
        failed |= !cpu_page_delivery_case(scenario);
    failed |= !cpu_divide_return_matrix();
    failed |= !cpu_aam_return_matrix();
    if (failed) return 1;
    if (cpu_ud_cache_preservation() || cpu_interrupt_pending_and_rollback()) return 1;
    lib_c_printf("%s\n", "M5:T539:S91:CPU-EXECUTION-FAULT-EVENT:OK");
    return 0;
}
