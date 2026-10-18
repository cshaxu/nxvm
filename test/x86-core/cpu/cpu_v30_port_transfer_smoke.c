#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"

typedef struct v30_port_record {
    lib_u16 port;
    lib_u8 bytes;
    lib_bool write, accepted, completed;
    lib_u32 value;
} v30_port_record;

typedef struct v30_port_fixture {
    cpu_bus_fixture bus;
    lib_u8 memory[0x100000u], ports[0x10000u];
    v30_port_record records[32];
    x86_core_cpu_external_cycle_phase phases[64];
    lib_u32 transfers, completions, phase_count, reads, writes;
    lib_u32 fail_port, fail_read, fail_write, fail_fetch, fetches;
    lib_u32 last_read, last_write;
    lib_bool invalid;
} v30_port_fixture;

static lib_status v30_port_read(void *opaque, lib_u32 address, void *data,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe, lib_bool reset_fetch, lib_u8 cache_control)
{
    v30_port_fixture *fixture = opaque;
    (void)reset_fetch;
    (void)cache_control;
    if (address >= sizeof(fixture->memory) || bytes > sizeof(fixture->memory) - address)
        return LIB_STATUS_IO_ERROR;
    if (!observe && provenance != X86_CORE_CPU_MEMORY_ACCESS_DATA &&
        ++fixture->fetches == fixture->fail_fetch) return LIB_STATUS_IO_ERROR;
    if (!observe && provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) {
        fixture->last_read = address;
        if (++fixture->reads == fixture->fail_read) return LIB_STATUS_IO_ERROR;
    }
    lib_memory_copy(data, fixture->memory + address, bytes);
    return LIB_STATUS_OK;
}

static lib_status v30_port_write(void *opaque, lib_u32 address, const void *data,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    v30_port_fixture *fixture = opaque;
    (void)provenance;
    (void)cache_control;
    fixture->last_write = address;
    if (address >= sizeof(fixture->memory) || bytes > sizeof(fixture->memory) - address ||
        ++fixture->writes == fixture->fail_write) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(fixture->memory + address, data, bytes);
    return LIB_STATUS_OK;
}

static lib_status v30_port_transfer(void *opaque, lib_u16 port, lib_u8 bytes,
    lib_bool write, lib_u32 *value)
{
    v30_port_fixture *fixture = opaque;
    const lib_bool accepted = fixture->transfers + 1u != fixture->fail_port;
    if (fixture->transfers >= 32u || bytes == 0u || bytes > 2u ||
        (bytes == 2u && (port & 1u)) || fixture->phase_count != fixture->transfers * 2u + 1u) {
        fixture->invalid = LIB_TRUE;
        return LIB_STATUS_IO_ERROR;
    }
    if (!write) *value = fixture->ports[port] |
        (bytes == 2u ? (lib_u32)fixture->ports[(lib_u16)(port + 1u)] << 8u : 0u);
    fixture->records[fixture->transfers++] = (v30_port_record){
        port, bytes, write, accepted, LIB_FALSE, *value};
    if (!accepted) return LIB_STATUS_IO_ERROR;
    if (write) {
        fixture->ports[port] = (lib_u8)*value;
        if (bytes == 2u) fixture->ports[(lib_u16)(port + 1u)] = (lib_u8)(*value >> 8u);
    }
    return LIB_STATUS_OK;
}

static void v30_port_complete(void *opaque, lib_u16 port, lib_u8 bytes, lib_bool write)
{
    v30_port_fixture *fixture = opaque;
    if (!fixture->transfers) { fixture->invalid = LIB_TRUE; return; }
    v30_port_record *record = &fixture->records[fixture->transfers - 1u];
    fixture->invalid |= record->port != port || record->bytes != bytes || record->write != write ||
        !record->accepted || record->completed || !fixture->bus.instructions.data.flagIgnore ||
        fixture->phase_count != fixture->transfers * 2u - 1u;
    record->completed = LIB_TRUE;
    ++fixture->completions;
}

static lib_u64 v30_port_cycle(void *opaque, x86_core_cpu_external_cycle_phase phase,
    x86_core_cpu_external_cycle_space space, lib_u32 address, lib_u8 bytes,
    lib_u8 write, x86_core_cpu_memory_access_provenance provenance)
{
    v30_port_fixture *fixture = opaque;
    (void)provenance;
    if (space != X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT) return 0u;
    if (fixture->phase_count >= 64u) { fixture->invalid = LIB_TRUE; return 0u; }
    fixture->phases[fixture->phase_count++] = phase;
    if (phase != X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN) {
        if (!fixture->transfers) { fixture->invalid = LIB_TRUE; return 0u; }
        const v30_port_record *record = &fixture->records[fixture->transfers - 1u];
        fixture->invalid |= record->port != address || record->bytes != bytes || record->write != write ||
            phase != (record->accepted ? X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT :
                X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL) || record->completed != record->accepted;
    }
    return 0u;
}

static void v30_port_prepare(v30_port_fixture *fixture)
{
    static const x86_core_cpu_bus_provider provider = {
        .read_memory = v30_port_read, .write_memory = v30_port_write,
        .transfer_port = v30_port_transfer, .complete_port = v30_port_complete,
        .interrupt_pending = cpu_bus_pending, .acknowledge_interrupt = cpu_bus_acknowledge
    };
    lib_memory_set(fixture, 0, sizeof(*fixture));
    cpu_bus_prepare(&fixture->bus, X86_CORE_CPU_PROFILE_V30);
    fixture->bus.execution.bus = &provider;
    fixture->bus.execution.bus_context = fixture;
    fixture->bus.cpu.data.ds.base = 0x10000u;
    fixture->bus.cpu.data.ss.base = 0x20000u;
    fixture->bus.cpu.data.es.base = 0x30000u;
    fixture->bus.cpu.data.esi = 0xa55a0400u;
    fixture->bus.cpu.data.edi = 0x5aa00500u;
    fixture->bus.cpu.data.ecx = 0x76540003u;
    fixture->bus.cpu.data.eflags |= VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF;
    x86_core_cpu_execution_context_bind_external_cycle_provider(&fixture->bus.execution,
        v30_port_cycle, fixture);
    for (lib_u32 port = 0u; port < sizeof(fixture->ports); ++port)
        fixture->ports[port] = (lib_u8)(0x11u + port % 64u);
}

static lib_i32 v30_port_records(const v30_port_fixture *fixture, lib_u16 port,
    lib_u8 width, lib_bool write, lib_u32 iterations, lib_u32 failed)
{
    const lib_u32 parts = width == 2u && (port & 1u) ? 2u : 1u;
    const lib_u32 count = failed ? failed : parts * iterations;
    if (fixture->invalid || fixture->transfers != count ||
        fixture->completions != count - (failed != 0u) || fixture->phase_count != count * 2u) return 1;
    for (lib_u32 item = 0u; item < count; ++item) {
        const v30_port_record *record = &fixture->records[item];
        if (record->port != (lib_u16)(port + item % parts) ||
            record->bytes != (parts == 2u ? 1u : width) || record->write != write ||
            record->accepted != (item + 1u != failed) || record->completed != record->accepted ||
            fixture->phases[item * 2u] != X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN ||
            fixture->phases[item * 2u + 1u] != (record->accepted ?
                X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT : X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL)) return 1;
    }
    return 0;
}

static lib_i32 v30_port_scalars(void)
{
    static const lib_u8 opcodes[] = {0xe4u,0xe5u,0xe6u,0xe7u,0xecu,0xedu,0xeeu,0xefu};
    static const lib_u16 ports[] = {0x20u,0x21u,0xffu,0xffffu};
    static const lib_u8 prefixes[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
    static v30_port_fixture fixture;
    for (lib_size form = 0u; form < sizeof(opcodes); ++form)
        for (lib_size item = 0u; item < sizeof(ports) / sizeof(ports[0]); ++item)
        for (lib_size prefix = 0u; prefix < sizeof(prefixes); ++prefix) {
            lib_u8 code[3], size=0u;
            if (prefix) code[size++]=prefixes[prefix];
            code[size++]=opcodes[form];
            if (form<4u) code[size++]=(lib_u8)ports[item];
            const lib_u16 port = form < 4u ? (lib_u8)ports[item] : ports[item];
            const lib_u8 width = (form & 1u) + 1u;
            const lib_bool write = (form & 2u) != 0u;
            v30_port_prepare(&fixture);
            fixture.bus.cpu.data.dx = ports[item];
            const t_cpu before = fixture.bus.cpu;
            const lib_u32 mask = width == 1u ? 0xffu : 0xffffu;
            const lib_u32 value = write ? before.data.eax & mask : fixture.ports[port] |
                (width == 2u ? (lib_u32)fixture.ports[(lib_u16)(port + 1u)] << 8u : 0u);
            lib_memory_copy(fixture.memory + 0x100u, code, size);
            x86_core_cpu_instruction_lexeme lexeme;
            if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution, &lexeme) ||
                lexeme.byte_count != size || fixture.transfers || fixture.fetches ||
                lib_memory_compare(&fixture.bus.cpu, &before, sizeof(before)) != 0) return 1;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            const lib_bool odd=width==2u && (port & 1u);
            const lib_u64 clocks=prefix ? 0u : (form<2u ? 9u : 8u)+(odd ? 4u : 0u);
            const lib_u32 key=clocks ? opcodes[form] : X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
            const x86_core_cpu_timing_result timing=x86_core_cpu_capture_timing(&fixture.bus.execution);
            if (timing.ticks!=clocks || !timing.source_timing_unallocated || timing.event_ticks ||
                timing.key_id!=key || timing.form_id!=key ||
                timing.formula_inputs!=(clocks && odd ? X86_CORE_CPU_TIMING_INPUT_ODD_WORD : 0u) ||
                timing.retirement_origin!=(clocks ? X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE :
                    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED)) return 1;
            if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.eip != 0x100u+size ||
                fixture.bus.cpu.data.eax != (write ? before.data.eax : (before.data.eax & ~mask) | value) ||
                fixture.bus.cpu.data.eflags != before.data.eflags || fixture.bus.cpu.data.edx != before.data.edx ||
                fixture.bus.cpu.data.ecx != before.data.ecx || fixture.reads || fixture.writes ||
                v30_port_records(&fixture, port, width, write, 1u, 0u)) return 1;
            lib_u32 actual = 0u;
            for (lib_u32 part = 0u; part < fixture.transfers; ++part)
                actual |= fixture.records[part].value << (part * 8u);
            if (actual != value) return 1;
            for (lib_u8 fetch=1u; fetch<=size; ++fetch) {
                v30_port_prepare(&fixture);
                fixture.bus.cpu.data.dx=ports[item];
                const t_cpu initial=fixture.bus.cpu;
                lib_memory_copy(fixture.memory+0x100u,code,size);
                fixture.fail_fetch=fetch;
                x86_core_cpu_execution_refresh(&fixture.bus.execution);
                if (!fixture.bus.execution.stop_requested || !fixture.bus.faults || fixture.transfers ||
                    fixture.bus.execution.completed_observation_valid || fixture.bus.execution.timing_result.ticks ||
                    fixture.bus.execution.timing_result.retirement_origin!=X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
                    lib_memory_compare(&fixture.bus.cpu,&initial,sizeof(initial))) return 1;
            }
        }
    return 0;
}

static lib_i32 v30_port_strings(void)
{
    static const lib_u8 prefixes[] = {0u,0xf3u};
    static const lib_u8 segments[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
    static const lib_u32 bases[] = {0x10000u,0x30000u,0u,0x20000u,0x10000u};
    static v30_port_fixture fixture;
    for (lib_u8 opcode = 0x6cu; opcode <= 0x6fu; ++opcode)
        for (lib_u8 repeat = 0u; repeat < 2u; ++repeat)
            for (lib_u8 segment = 0u; segment < 5u; ++segment)
                for (lib_u8 mode = 0u; mode < 8u; ++mode) {
                    const lib_bool write = (opcode & 2u) != 0u;
                    const lib_u8 width = (opcode & 1u) + 1u;
                    const lib_bool direction = (mode & 4u) != 0u;
                    const lib_u16 port = 0x20u + (mode & 1u);
                    const lib_u16 index = (mode & 2u) ? 0xffffu : 0x400u;
                    const lib_u32 base = write ? bases[segment] : 0x30000u;
                    const lib_u16 count = repeat ? 3u : 1u;
                    lib_u8 code[3], size = 0u;
                    if (prefixes[repeat]) code[size++] = prefixes[repeat];
                    if (segments[segment]) code[size++] = segments[segment];
                    code[size++] = opcode;
                    v30_port_prepare(&fixture);
                    fixture.bus.cpu.data.dx = port;
                    fixture.bus.cpu.data.si = fixture.bus.cpu.data.di = index;
                    if (direction) fixture.bus.cpu.data.eflags |= VCPU_EFLAGS_DF;
                    const t_cpu before = fixture.bus.cpu;
                    const lib_u16 value = write ? 0x7856u : (lib_u16)(fixture.ports[port] |
                        (lib_u16)fixture.ports[port + 1u] << 8u);
                    for (lib_u16 iteration = 0u; iteration < count; ++iteration) {
                        const lib_u16 offset = index + (direction ? 0u - iteration * width : iteration * width);
                        if (write) lib_memory_copy(fixture.memory + base + offset, &value, width);
                    }
                    lib_memory_copy(fixture.memory + 0x100u, code, size);
                    for (lib_u16 iteration = 0u; iteration < count; ++iteration) {
                        const lib_u16 offset = index + (direction ? 0u - iteration * width : iteration * width);
                        x86_core_cpu_execution_refresh(&fixture.bus.execution);
                        const lib_u16 next = index + (direction ? 0u - (iteration + 1u) * width : (iteration + 1u) * width);
                        lib_u16 actual = 0u;
                        lib_memory_copy(&actual, fixture.memory + base + offset, width);
                        if (fixture.bus.execution.stop_requested || actual != (width == 1u ? value & 0xffu : value) ||
                            fixture.bus.cpu.data.eip != (iteration + 1u == count ? 0x100u + size : 0x100u) ||
                            fixture.bus.cpu.data.ecx != (repeat ? before.data.ecx - iteration - 1u : before.data.ecx) ||
                            fixture.bus.cpu.data.esi != ((before.data.esi & 0xffff0000u) | (write ? next : index)) ||
                            fixture.bus.cpu.data.edi != ((before.data.edi & 0xffff0000u) | (write ? index : next)) ||
                            fixture.bus.cpu.data.eax != before.data.eax || fixture.bus.cpu.data.edx != before.data.edx ||
                            fixture.bus.cpu.data.eflags != before.data.eflags ||
                            (write ? fixture.last_read : fixture.last_write) != base + offset +
                                (width == 2u && (offset & 1u) ? 1u : 0u)) return 1;
                    }
                    if ((write ? fixture.writes != 0u : fixture.reads != 0u) ||
                        (write ? fixture.reads : fixture.writes) != count *
                            (width == 2u && (index & 1u) ? 2u : 1u) ||
                        v30_port_records(&fixture, port, width, write, count, 0u)) return 1;
                    for (lib_u32 part = 0u; part < fixture.transfers; ++part)
                        if (fixture.records[part].value != (width == 2u && (port & 1u) ?
                            (value >> ((part % 2u) * 8u)) & 0xffu : width == 1u ? value & 0xffu : value)) return 1;
                }
    return 0;
}

static lib_i32 v30_port_failures(void)
{
    static const lib_u8 opcodes[] = {
        0xe4u,0xe5u,0xe6u,0xe7u,0xecu,0xedu,0xeeu,0xefu,0x6cu,0x6du,0x6eu,0x6fu
    };
    static v30_port_fixture fixture;
    for (lib_size form = 0u; form < sizeof(opcodes); ++form)
        for (lib_u32 stage = 1u; stage <= (lib_u32)((form & 1u) + 1u); ++stage) {
            const lib_bool write = (form & 2u) != 0u;
            const lib_u8 width = (form & 1u) + 1u;
            const lib_u8 code[] = {opcodes[form],0x21u};
            v30_port_prepare(&fixture);
            fixture.bus.cpu.data.dx = 0x21u;
            fixture.fail_port = stage;
            lib_memory_copy(fixture.memory + 0x10400u, &fixture.bus.cpu.data.ax, 2u);
            const t_cpu before = fixture.bus.cpu;
            lib_memory_copy(fixture.memory + 0x100u, code, sizeof(code));
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || fixture.bus.delivered_exceptions ||
                fixture.bus.execution.completed_observation_valid || fixture.bus.execution.timing_result.ticks ||
                fixture.bus.execution.timing_result.retirement_origin!=X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
                fixture.bus.cpu.data.eip != before.data.eip || fixture.bus.cpu.data.eax != before.data.eax ||
                fixture.bus.cpu.data.esi != before.data.esi || fixture.bus.cpu.data.edi != before.data.edi ||
                fixture.bus.cpu.data.ecx != before.data.ecx || fixture.writes ||
                v30_port_records(&fixture, 0x21u, width, write, 1u, stage) ||
                fixture.ports[0x21u] != (write && stage == 2u ? (lib_u8)before.data.ax : 0x32u) ||
                fixture.ports[0x22u] != 0x33u) return 1;
        }
    for (lib_u8 write = 0u; write < 2u; ++write)
        for (lib_u32 stage = 1u; stage <= 2u; ++stage) {
            const lib_u8 code[] = {0xf3u,write ? 0x6fu : 0x6du};
            v30_port_prepare(&fixture);
            fixture.bus.cpu.data.dx = 0x21u;
            fixture.bus.cpu.data.si = fixture.bus.cpu.data.di = 0xffffu;
            lib_memory_copy(fixture.memory + 0x1ffffu, (const lib_u8[]){0x78u,0x56u}, 2u);
            if (write) fixture.fail_read = stage;
            else fixture.fail_write = stage;
            lib_memory_copy(fixture.memory + 0x100u, code, sizeof(code));
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || fixture.bus.cpu.data.cx != 3u ||
                fixture.bus.cpu.data.eip != 0x100u || fixture.bus.cpu.data.si != 0xffffu ||
                fixture.bus.cpu.data.di != 0xffffu || fixture.bus.delivered_exceptions ||
                (write ? fixture.reads : fixture.writes) != stage ||
                (write ? fixture.transfers != 0u : v30_port_records(&fixture, 0x21u, 2u, LIB_FALSE, 1u, 0u)) ||
                fixture.memory[0x3ffffu] != (!write && stage == 2u ? 0x32u : 0u) ||
                fixture.memory[0x40000u] != 0u) return 1;
        }
    return 0;
}

static lib_i32 v30_port_repeat_and_entry(void)
{
    static v30_port_fixture fixture;
    for (lib_u8 opcode = 0x6cu; opcode <= 0x6fu; ++opcode) {
        const lib_u8 code[] = {0xf3u,opcode};
        v30_port_prepare(&fixture);
        fixture.bus.cpu.data.cx = 0u;
        lib_memory_copy(fixture.memory + 0x100u, code, sizeof(code));
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.eip != 0x102u ||
            fixture.transfers || fixture.reads || fixture.writes || fixture.bus.cpu.data.si != 0x400u ||
            fixture.bus.cpu.data.di != 0x500u) return 1;
        for (lib_u8 prefix = 0x64u; prefix <= 0x65u; ++prefix) {
            const lib_u8 rejected[] = {prefix,opcode};
            x86_core_cpu_instruction_lexeme lexeme;
            v30_port_prepare(&fixture);
            lib_memory_copy(fixture.memory + 0x100u, rejected, sizeof(rejected));
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || fixture.bus.cpu.data.eip != 0x100u ||
                fixture.transfers || fixture.reads || fixture.writes ||
                x86_core_cpu_instruction_lexeme_scan(rejected, sizeof(rejected),
                    X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme)) return 1;
        }
        for (lib_u8 nmi = 0u; nmi < 2u; ++nmi) {
            v30_port_prepare(&fixture);
            fixture.bus.cpu.data.cx = 2u;
            fixture.bus.cpu.data.dx = 0x21u;
            fixture.bus.cpu.data.eflags |= VCPU_EFLAGS_IF;
            fixture.memory[nmi ? 0x09u : 0xc1u] = 2u;
            fixture.memory[0x200u] = 0xcfu;
            lib_memory_copy(fixture.memory + 0x100u, code, sizeof(code));
            if (nmi) { if (!x86_core_cpu_request_nmi(&fixture.bus.execution)) return 1; }
            else fixture.bus.interrupt = LIB_TRUE;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            lib_u16 saved_ip = 0u;
            lib_memory_copy(&saved_ip, fixture.memory + 0x206fau, 2u);
            const lib_u32 first = fixture.transfers;
            if (fixture.bus.execution.stop_requested || saved_ip != 0x100u || fixture.bus.cpu.data.cx != 1u ||
                fixture.bus.cpu.data.eip != 0x200u || fixture.bus.acknowledgements != (lib_u32)!nmi) return 1;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (fixture.bus.execution.stop_requested || fixture.transfers != first || fixture.bus.cpu.data.eip != 0x100u ||
                fixture.bus.cpu.data.sp != 0x700u || !(fixture.bus.cpu.data.eflags & X86_CPU_NEC_PSW_MD)) return 1;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.cx || fixture.bus.cpu.data.eip != 0x102u ||
                v30_port_records(&fixture, 0x21u, (opcode & 1u) + 1u, (opcode & 2u) != 0u, 2u, 0u)) return 1;
        }
    }
    return 0;
}

static lib_i32 v30_port_repeat_failure(void)
{
    static v30_port_fixture fixture;
    for (lib_u8 opcode = 0x6cu; opcode <= 0x6fu; ++opcode)
        for (lib_u32 stage = 1u; stage <= (lib_u32)((opcode & 1u) + 1u); ++stage) {
            const lib_u8 code[] = {0xf3u,opcode};
            const lib_u8 width = (opcode & 1u) + 1u;
            const lib_bool write = (opcode & 2u) != 0u;
            v30_port_prepare(&fixture);
            fixture.bus.cpu.data.dx = 0x21u;
            lib_memory_copy(fixture.memory + 0x10400u, (const lib_u8[]){0x12u,0x34u,0x56u,0x78u}, 4u);
            lib_memory_copy(fixture.memory + 0x100u, code, sizeof(code));
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            const t_cpu before = fixture.bus.cpu;
            if (fixture.bus.execution.stop_requested || before.data.eip != 0x100u || before.data.cx != 2u) return 1;
            fixture.fail_port = fixture.transfers + stage;
            const lib_u32 completed = fixture.completions;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || fixture.bus.delivered_exceptions ||
                fixture.bus.cpu.data.eip != before.data.eip || fixture.bus.cpu.data.ecx != before.data.ecx ||
                fixture.bus.cpu.data.esi != before.data.esi || fixture.bus.cpu.data.edi != before.data.edi ||
                fixture.bus.cpu.data.eax != before.data.eax || fixture.bus.cpu.data.eflags != before.data.eflags ||
                fixture.completions != completed + stage - 1u ||
                v30_port_records(&fixture, 0x21u, width, write, 2u, width + stage) ||
                (write ? fixture.writes != 0u : fixture.writes != 1u) ||
                (!write && fixture.memory[0x30500u + width] != 0u)) return 1;
            if (write && fixture.ports[0x21u] != (stage == 2u ? 0x56u : 0x12u)) return 1;
        }
    return 0;
}

lib_i32 main(void)
{
    if (v30_port_scalars()) { lib_c_printf("V30:PORT-SCALARS:FAIL\n"); return 1; }
    if (v30_port_strings()) { lib_c_printf("V30:PORT-STRINGS:FAIL\n"); return 1; }
    if (v30_port_failures()) { lib_c_printf("V30:PORT-FAILURES:FAIL\n"); return 1; }
    if (v30_port_repeat_and_entry()) { lib_c_printf("V30:PORT-ENTRY:FAIL\n"); return 1; }
    if (v30_port_repeat_failure()) { lib_c_printf("V30:PORT-REPEAT-FAILURE:FAIL\n"); return 1; }
    lib_c_printf("V30:PORT-TRANSFER:OK\n");
    lib_c_printf("V30:NOMINAL-PORT:OK cases=160\n");
    return 0;
}
