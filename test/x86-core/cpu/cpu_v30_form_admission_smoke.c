#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

typedef struct v30_form_fixture {
    cpu_instruction_fixture instruction;
    lib_u32 reads, writes, ports, observations, fetches;
    lib_u32 failed_code_address;
    lib_bool refuse_code;
} v30_form_fixture;

static lib_status v30_form_read(void *opaque, lib_u32 address, void *data,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe, lib_bool reset_fetch, lib_u8 cache_control)
{
    v30_form_fixture *fixture = opaque;
    if (observe) ++fixture->observations;
    else if (provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) ++fixture->reads;
    else ++fixture->fetches;
    if (fixture->refuse_code && provenance != X86_CORE_CPU_MEMORY_ACCESS_DATA &&
        address <= fixture->failed_code_address && fixture->failed_code_address - address < bytes)
        return LIB_STATUS_IO_ERROR;
    return cpu_instruction_read(opaque, address, data, bytes, provenance, observe, reset_fetch, cache_control);
}

static lib_status v30_form_write(void *opaque, lib_u32 address, const void *data,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    ++((v30_form_fixture *)opaque)->writes;
    return cpu_instruction_write(opaque, address, data, bytes, provenance, cache_control);
}

static lib_status v30_form_port(void *opaque, lib_u16 port, lib_u8 bytes,
    lib_bool write, lib_u32 *value)
{
    (void)port;
    (void)bytes;
    (void)write;
    (void)value;
    ++((v30_form_fixture *)opaque)->ports;
    return LIB_STATUS_IO_ERROR;
}

static void v30_form_prepare(v30_form_fixture *fixture)
{
    static const x86_core_cpu_bus_provider provider = {
        .read_memory = v30_form_read, .write_memory = v30_form_write,
        .transfer_port = v30_form_port, .interrupt_pending = cpu_instruction_interrupt_pending
    };
    lib_memory_set(fixture, 0, sizeof(*fixture));
    cpu_instruction_prepare_with_bus(&fixture->instruction, X86_CORE_CPU_PROFILE_V30, &provider, fixture);
    fixture->instruction.cpu.data.sp = 0x800u;
    fixture->instruction.cpu.data.ax = 1u;
    fixture->instruction.cpu.data.cx = 1u;
}

/* Independent shape expectations from the documented native encoding fields. */
static lib_bool v30_form_expected(lib_u8 opcode, lib_u8 modrm)
{
    const lib_u8 operation = (modrm / 8u) % 8u;
    const lib_bool reg = modrm >= 0xc0u;
    switch (opcode) {
    case 0x62u: case 0x8du: case 0xc4u: case 0xc5u: return !reg;
    case 0x8cu: return operation < 4u;
    case 0x8eu: return operation < 4u && operation != 1u;
    case 0x8fu: case 0xc6u: case 0xc7u: return operation == 0u;
    case 0xc0u: case 0xc1u: case 0xd0u: case 0xd1u: case 0xd2u: case 0xd3u:
        return operation != 6u;
    case 0xf6u: case 0xf7u: return operation != 1u;
    case 0xfeu: return operation <= 1u;
    case 0xffu: return operation != 7u && (!reg || (operation != 3u && operation != 5u));
    default: return LIB_FALSE;
    }
}

static lib_i32 v30_form_matrix(void)
{
    static const lib_u8 opcodes[] = {
        0x62u,0x8cu,0x8du,0x8eu,0x8fu,0xc0u,0xc1u,0xc4u,0xc5u,0xc6u,0xc7u,
        0xd0u,0xd1u,0xd2u,0xd3u,0xf6u,0xf7u,0xfeu,0xffu
    };
    static const lib_u8 prefixes[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
    static v30_form_fixture fixture;
    for (lib_size op = 0u; op < sizeof(opcodes); ++op)
        for (lib_u16 form = 0u; form <= 255u; ++form)
            for (lib_u8 prefix = 0u; prefix < 5u; ++prefix) {
                lib_u8 code[10] = {0};
                const lib_u8 start = prefix != 0u;
                if (prefix) code[0] = prefixes[prefix];
                code[start] = opcodes[op];
                code[start + 1u] = (lib_u8)form;
                x86_core_cpu_instruction_lexeme lexeme;
                const lib_bool expected = v30_form_expected(opcodes[op], (lib_u8)form);
                if (x86_core_cpu_instruction_lexeme_scan(code, sizeof(code), X86_CORE_CPU_PROFILE_V30,
                    LIB_FALSE, &lexeme) != expected) {
                    lib_c_printf("V30:FORM-SCAN:%02x:%02x\n",opcodes[op],form);
                    return 1;
                }
                if (expected) continue;
                v30_form_prepare(&fixture);
                const t_cpu before = fixture.instruction.cpu;
                lib_memory_copy(fixture.instruction.memory, code, sizeof(code));
                if (x86_core_cpu_execution_preview_lexeme(&fixture.instruction.execution, &lexeme) ||
                    fixture.reads || fixture.writes || fixture.ports ||
                    lib_memory_compare(&fixture.instruction.cpu, &before, sizeof(before)) != 0) return 1;
                x86_core_cpu_execution_refresh(&fixture.instruction.execution);
                if (!fixture.instruction.execution.stop_requested || fixture.instruction.delivered_exception.valid ||
                    fixture.reads || fixture.writes || fixture.ports ||
                    lib_memory_compare(&fixture.instruction.cpu, &before, sizeof(before)) != 0) {
                    lib_c_printf("V30:FORM-RUNTIME:%02x:%02x:%u:%u:%u\n",
                        opcodes[op],form,prefix,fixture.reads,fixture.writes);
                    return 1;
                }
            }
    return 0;
}

static lib_i32 v30_form_witnesses(void)
{
    static const struct {
        lib_u8 code[6], size;
        lib_u16 ax, memory, sp;
        lib_u32 pc;
    } cases[] = {
        {{0x8du,0x06u,0u,4u},4u,0x400u,0x200u,0x800u,4u},
        {{0xc7u,0x06u,0u,4u,0x34u,0x12u},6u,1u,0x1234u,0x800u,6u},
        {{0xc6u,0x06u,0u,4u,0x34u},5u,1u,0x234u,0x800u,5u},
        {{0x8fu,0x06u,0u,4u},4u,1u,0x5678u,0x802u,4u},
        {{0xc4u,0x06u,0u,4u},4u,0x200u,0x200u,0x800u,4u},
        {{0xc5u,0x06u,0u,4u},4u,0x200u,0x200u,0x800u,4u},
        {{0xffu,0x06u,0u,4u},4u,1u,0x201u,0x800u,4u},
        {{0xfeu,0x0eu,0u,4u},4u,1u,0x2ffu,0x800u,4u},
        {{0xffu,0x16u,0u,4u},4u,1u,0x200u,0x7feu,0x200u},
        {{0xffu,0x1eu,0u,4u},4u,1u,0x200u,0x7fcu,0x200u},
        {{0xffu,0x26u,0u,4u},4u,1u,0x200u,0x800u,0x200u},
        {{0xffu,0x2eu,0u,4u},4u,1u,0x200u,0x800u,0x200u},
        {{0xffu,0x36u,0u,4u},4u,1u,0x200u,0x7feu,4u},
        {{0xf6u,0x16u,0u,4u},4u,1u,0x2ffu,0x800u,4u},
        {{0xf7u,0xd0u},2u,0xfffeu,0x200u,0x800u,2u},
        {{0xd3u,0x26u,0u,4u},4u,1u,0x400u,0x800u,4u},
        {{0x8cu,0xc0u},2u,0u,0x200u,0x800u,2u},
        {{0x8eu,0xd8u},2u,1u,0x200u,0x800u,2u},
        {{0x62u,0x06u,0u,4u},4u,1u,0u,0x800u,4u}
    };
    static v30_form_fixture fixture;
    for (lib_size item = 0u; item < sizeof(cases) / sizeof(cases[0]); ++item) {
        x86_core_cpu_instruction_lexeme lexeme;
        for (lib_size length = 0u; length < cases[item].size; ++length)
            if (x86_core_cpu_instruction_lexeme_scan(cases[item].code, length,
                X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme)) return 1;
        if (!x86_core_cpu_instruction_lexeme_scan(cases[item].code, cases[item].size,
            X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme)) return 1;
        v30_form_prepare(&fixture);
        fixture.instruction.memory[0x401u] = 2u;
        fixture.instruction.memory[0x800u] = 0x78u;
        fixture.instruction.memory[0x801u] = 0x56u;
        if (cases[item].code[0] == 0x62u) {
            fixture.instruction.memory[0x401u] = 0u;
            fixture.instruction.memory[0x402u] = 10u;
        }
        lib_memory_copy(fixture.instruction.memory, cases[item].code, cases[item].size);
        x86_core_cpu_execution_refresh(&fixture.instruction.execution);
        lib_u16 memory = 0u;
        lib_memory_copy(&memory, fixture.instruction.memory + 0x400u, 2u);
        if (fixture.instruction.execution.stop_requested || fixture.instruction.delivered_exception.valid ||
            fixture.instruction.cpu.data.eip != cases[item].pc || fixture.instruction.cpu.data.ax != cases[item].ax ||
            memory != cases[item].memory || fixture.instruction.cpu.data.sp != cases[item].sp) {
            lib_c_printf("V30:FORM-WITNESS:%u\n",(lib_u32)item);
            return 1;
        }
    }
    return 0;
}

static lib_i32 v30_form_fetch(void)
{
    static v30_form_fixture fixture;
    static const lib_u8 opcodes[] = {0xd1u,0xc7u,0xffu,0x8eu};
    static const lib_u8 invalid[] = {0x36u,0x0eu,0x3eu,0x0eu};
    for (lib_size item = 0u; item < sizeof(opcodes); ++item) {
        v30_form_prepare(&fixture);
        fixture.instruction.cpu.data.eip = 0xffffu;
        fixture.instruction.memory[0xffffu] = opcodes[item];
        fixture.instruction.memory[0u] = invalid[item];
        fixture.instruction.memory[1u] = 0u;
        fixture.instruction.memory[2u] = 4u;
        const t_cpu before = fixture.instruction.cpu;
        x86_core_cpu_execution_refresh(&fixture.instruction.execution);
        if (!fixture.instruction.execution.stop_requested || fixture.reads || fixture.writes ||
            fixture.instruction.delivered_exception.valid ||
            lib_memory_compare(&fixture.instruction.cpu, &before, sizeof(before)) != 0) return 1;
        v30_form_prepare(&fixture);
        fixture.instruction.cpu.data.eip = 0xffffu;
        fixture.instruction.memory[0xffffu] = opcodes[item];
        fixture.instruction.memory[0u] = 6u;
        fixture.refuse_code = LIB_TRUE;
        fixture.failed_code_address = 0u;
        x86_core_cpu_execution_refresh(&fixture.instruction.execution);
        if (!fixture.instruction.execution.stop_requested || fixture.instruction.cpu.data.eip != 0xffffu ||
            fixture.reads || fixture.writes || fixture.instruction.delivered_exception.valid) return 1;
    }
    v30_form_prepare(&fixture);
    fixture.instruction.cpu.data.eip = 0xffffu;
    fixture.instruction.memory[0xffffu] = 0xc7u;
    lib_memory_copy(fixture.instruction.memory, (const lib_u8[]){6u,0u,4u,0x34u,0x12u}, 5u);
    x86_core_cpu_execution_refresh(&fixture.instruction.execution);
    lib_u16 value = 0u;
    lib_memory_copy(&value, fixture.instruction.memory + 0x400u, 2u);
    return fixture.instruction.execution.stop_requested || value != 0x1234u ||
        fixture.instruction.cpu.data.eip != 5u || fixture.reads || fixture.writes != 1u;
}

lib_i32 main(void)
{
    if (v30_form_matrix()) return 1;
    if (v30_form_witnesses()) return 1;
    if (v30_form_fetch()) { lib_c_printf("V30:FORM-FETCH:FAIL\n"); return 1; }
    lib_c_printf("V30:FORM-ADMISSION:OK\n");
    return 0;
}
