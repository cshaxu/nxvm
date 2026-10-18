#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

#define V30_ALU_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF)

typedef enum v30_alu_operation {
    V30_ALU_ADD, V30_ALU_OR, V30_ALU_ADC, V30_ALU_SBB,
    V30_ALU_AND, V30_ALU_SUB, V30_ALU_XOR, V30_ALU_CMP,
    V30_ALU_TEST, V30_ALU_INC, V30_ALU_DEC, V30_ALU_NEG,
    V30_ALU_NOT, V30_ALU_CBW, V30_ALU_CWD
} v30_alu_operation;

typedef enum v30_alu_form {
    V30_ALU_RM_REGISTER, V30_ALU_REGISTER_RM, V30_ALU_ACC_IMMEDIATE,
    V30_ALU_GROUP_IMMEDIATE, V30_ALU_GROUP_SIGNED, V30_ALU_UNARY_GROUP,
    V30_ALU_UNARY_REGISTER, V30_ALU_EXTEND
} v30_alu_form;

typedef struct v30_alu_case {
    v30_alu_operation operation;
    v30_alu_form form;
    lib_u8 code[8], length, width, modrm, prefix;
    lib_u16 immediate, left;
    lib_bool carry, seed_accumulator;
} v30_alu_case;

static lib_u32 *v30_alu_word(t_cpu *cpu, lib_u8 reg)
{
    lib_u32 *words[] = {&cpu->data.eax,&cpu->data.ecx,&cpu->data.edx,&cpu->data.ebx,
        &cpu->data.esp,&cpu->data.ebp,&cpu->data.esi,&cpu->data.edi};
    return words[reg];
}

static lib_u16 v30_alu_get(t_cpu *cpu, lib_u8 reg, lib_u8 width)
{
    const lib_u32 value = *v30_alu_word(cpu, width == 2u ? reg : reg & 3u);
    return width == 2u ? (lib_u16)value : (lib_u8)(value >> (reg < 4u ? 0u : 8u));
}

static void v30_alu_set(t_cpu *cpu, lib_u8 reg, lib_u8 width, lib_u16 value)
{
    const lib_u8 shift = width == 1u && reg >= 4u ? 8u : 0u;
    const lib_u32 mask = (width == 2u ? 0xffffu : 0xffu) << shift;
    lib_u32 *word = v30_alu_word(cpu, width == 2u ? reg : reg & 3u);
    *word = (*word & ~mask) | (((lib_u32)value << shift) & mask);
}

/* Wide signed ranges and nibble arithmetic are independent of CPU lazy flags. */
static lib_u16 v30_alu_oracle(v30_alu_operation operation, lib_u8 width,
    lib_u16 left, lib_u16 right, lib_u32 *flags, lib_u32 *defined)
{
    const lib_u32 mask = width == 1u ? 255u : 65535u;
    left &= mask;
    right &= mask;
    const lib_i32 sign = (lib_i32)(mask / 2u + 1u);
    const lib_i32 a = left >= sign ? (lib_i32)left - (lib_i32)mask - 1 : left;
    const lib_i32 b = right >= sign ? (lib_i32)right - (lib_i32)mask - 1 : right;
    const lib_u32 carry = (*flags & VCPU_EFLAGS_CF) != 0u;
    const lib_bool subtract = operation == V30_ALU_SUB || operation == V30_ALU_SBB ||
        operation == V30_ALU_CMP || operation == V30_ALU_DEC || operation == V30_ALU_NEG;
    const lib_bool logical = operation == V30_ALU_OR || operation == V30_ALU_AND ||
        operation == V30_ALU_XOR || operation == V30_ALU_TEST;
    const lib_u32 cin = operation == V30_ALU_ADC || operation == V30_ALU_SBB ? carry : 0u;
    lib_u32 result, cf = 0u, af = 0u;
    lib_i32 signed_result = 0;
    *defined = LIB_UINT32_MAX;
    if (operation == V30_ALU_NOT) return (lib_u16)(left ^ mask);
    if (operation == V30_ALU_CBW) return (lib_u16)((left & 128u) ? left | 0xff00u : left);
    if (operation == V30_ALU_CWD) return left & 0x8000u ? 0xffffu : 0u;
    if (logical) {
        result = operation == V30_ALU_OR ? left | right :
            operation == V30_ALU_XOR ? left ^ right : left & right;
        *defined &= ~VCPU_EFLAGS_AF; /* NEC marks AC undefined for logical results. */
    } else {
        const lib_u32 x = operation == V30_ALU_NEG ? 0u : left;
        const lib_u32 y = operation == V30_ALU_INC || operation == V30_ALU_DEC ? 1u :
            operation == V30_ALU_NEG ? left : right;
        result = subtract ? x - y - cin : x + y + cin;
        cf = subtract ? x < y + cin : result > mask;
        af = subtract ? (x & 15u) < (y & 15u) + cin :
            (x & 15u) + (y & 15u) + cin > 15u;
        signed_result = operation == V30_ALU_NEG ? -a :
            operation == V30_ALU_INC ? a + 1 : operation == V30_ALU_DEC ? a - 1 :
            subtract ? a - b - (lib_i32)cin : a + b + (lib_i32)cin;
    }
    result &= mask;
    lib_u8 parity = 0u;
    for (lib_u8 bit = 0u; bit < 8u; ++bit) parity += (result >> bit) & 1u;
    *flags = (*flags & ~V30_ALU_FLAGS) | (cf ? VCPU_EFLAGS_CF : 0u) |
        (af ? VCPU_EFLAGS_AF : 0u) | (result == 0u ? VCPU_EFLAGS_ZF : 0u) |
        (result >= (lib_u32)sign ? VCPU_EFLAGS_SF : 0u) |
        (!(parity & 1u) ? VCPU_EFLAGS_PF : 0u) |
        (!logical && (signed_result < -sign || signed_result >= sign) ? VCPU_EFLAGS_OF : 0u);
    if (operation == V30_ALU_INC || operation == V30_ALU_DEC)
        *flags = (*flags & ~VCPU_EFLAGS_CF) | (carry ? VCPU_EFLAGS_CF : 0u);
    return (lib_u16)result;
}

static lib_u32 v30_alu_address(t_cpu *cpu, const v30_alu_case *item)
{
    const lib_u8 mod = item->modrm >> 6u, rm = item->modrm & 7u;
    const lib_u16 offsets[] = {cpu->data.bx + cpu->data.si, cpu->data.bx + cpu->data.di,
        cpu->data.bp + cpu->data.si, cpu->data.bp + cpu->data.di, cpu->data.si,
        cpu->data.di, cpu->data.bp, cpu->data.bx};
    const lib_bool stack = rm == 2u || rm == 3u || (rm == 6u && mod != 0u);
    const lib_u32 bases[] = {stack ? 0x20000u : 0x10000u,0x30000u,0x80000u,0x20000u,0x10000u};
    return bases[item->prefix] + (lib_u16)(mod == 0u && rm == 6u ? 0xffffu :
        offsets[rm] + (mod == 1u ? 0xff80u : mod == 2u ? 0xffffu : 0u));
}

static void v30_alu_prepare(v30_memory_fixture *fixture, const v30_alu_case *item)
{
    v30_memory_restart(fixture);
    for (lib_u8 reg = 0u; reg < 8u; ++reg)
        *v30_alu_word(&fixture->bus.cpu,reg) = 0x12340000u + reg * 0x1111u + 0x1987u;
    fixture->bus.cpu.data.eflags |= VCPU_EFLAGS_IF | VCPU_EFLAGS_DF |
        VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_PF;
    if (item->carry) fixture->bus.cpu.data.eflags |= VCPU_EFLAGS_CF;
    else fixture->bus.cpu.data.eflags &= ~VCPU_EFLAGS_CF;
    if (item->operation >= V30_ALU_INC && item->operation <= V30_ALU_NOT &&
        (item->modrm >> 6u) == 3u)
        v30_alu_set(&fixture->bus.cpu,item->modrm & 7u,item->width,item->left);
    else if (item->seed_accumulator) v30_alu_set(&fixture->bus.cpu,0u,item->width,item->left);
    if (item->form != V30_ALU_ACC_IMMEDIATE && (item->modrm >> 6u) != 3u && item->form < V30_ALU_EXTEND) {
        const lib_u32 address = v30_alu_address(&fixture->bus.cpu,item);
        fixture->memory[address] = (lib_u8)item->left;
        fixture->memory[address+1u] = (lib_u8)(item->left >> 8u);
        fixture->memory[address+2u] = 0xa5u;
    }
    lib_memory_copy(fixture->memory+0x80100u,item->code,item->length);
}

static lib_i32 v30_alu_records(const v30_memory_fixture *fixture, lib_u32 address,
    lib_u8 width, lib_bool memory, lib_bool write, lib_u16 initial, lib_u16 value,
    lib_bool failed)
{
    lib_u32 reads = 0u, writes = 0u, parts = 0u, bytes_read = 0u, bytes_written = 0u;
    lib_u16 expected = initial;
    lib_bool refused = LIB_FALSE;
    for (lib_u32 index = 0u; index < fixture->record_count; ++index) {
        const v30_memory_record *record = &fixture->records[index];
        if (record->observe || record->provenance != X86_CORE_CPU_MEMORY_ACCESS_DATA) continue;
        const lib_bool output = bytes_read == width;
        const lib_u32 offset = output ? bytes_written : bytes_read;
        const lib_u8 count = width == 2u && offset == 0u && !(address & 1u) ? 2u : 1u;
        const lib_u32 ordinal = output ? ++writes : ++reads;
        const lib_bool accepted = ordinal != (output ? fixture->fail_write : fixture->fail_read);
        if (!memory || refused || offset >= width || (output && !write) ||
            record->write != output || record->address != address+offset ||
            record->bytes != count || record->accepted != accepted ||
            fixture->phases[2u*parts] != X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN ||
            fixture->phases[2u*parts+1u] != (accepted ? X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT :
                X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL)) return 1;
        if (output && accepted) {
            const lib_u32 mask = (count == 2u ? 0xffffu : 0xffu) << (offset*8u);
            expected = (lib_u16)((expected & ~mask) | (value & mask));
        }
        if (output) bytes_written += count;
        else bytes_read += count;
        refused = !accepted;
        ++parts;
    }
    if (fixture->invalid || fixture->reads != reads || fixture->writes != writes ||
        fixture->phase_count != 2u*parts || refused != failed ||
        (!failed && (bytes_read != (memory ? width : 0u) || bytes_written != (write ? width : 0u)))) return 1;
    if (memory && (fixture->memory[address] != (lib_u8)expected ||
        fixture->memory[address+1u] != (lib_u8)(expected >> 8u) || fixture->memory[address+2u] != 0xa5u)) return 1;
    return 0;
}

/* NEC 12-29..41, 12-47..49 and 12-65..81, selected by semantic test
 * operation/form, independently of the production opcode-based selector. */
static lib_i32 v30_alu_timing(const v30_memory_fixture *fixture,
    const v30_alu_case *item, lib_bool memory, lib_u32 address)
{
    lib_u64 clocks = 0u;
    lib_u8 odd_cost = 0u;
    if (!item->prefix && item->form == V30_ALU_EXTEND) {
        /* CVTWL gives only a data-dependent range; CVTBW has a fixed row. */
        if (item->operation == V30_ALU_CBW) clocks = 2u;
    } else if (!item->prefix) {
        if (!memory) clocks = item->form >= V30_ALU_ACC_IMMEDIATE &&
            item->form <= V30_ALU_GROUP_SIGNED ? 4u : 2u;
        else if (item->form >= V30_ALU_UNARY_GROUP) { clocks = 16u; odd_cost = 8u; }
        else if (item->form == V30_ALU_GROUP_IMMEDIATE || item->form == V30_ALU_GROUP_SIGNED) {
            clocks = item->operation == V30_ALU_CMP ? 13u : item->operation == V30_ALU_TEST ? 11u : 18u;
            odd_cost = item->operation == V30_ALU_CMP || item->operation == V30_ALU_TEST ? 4u : 8u;
        } else if (item->operation == V30_ALU_TEST) { clocks = 10u; odd_cost = 4u; }
        else if (item->operation == V30_ALU_CMP || item->form == V30_ALU_REGISTER_RM) {
            clocks = 11u; odd_cost = 4u;
        } else { clocks = 16u; odd_cost = 8u; }
    }
    const lib_bool odd = clocks && odd_cost && item->width == 2u && (address & 1u);
    if (odd) clocks += odd_cost;
    lib_u32 form = X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    if (clocks) {
        form = item->code[0] | (memory ? 0x100u : 0u);
        if (item->form == V30_ALU_GROUP_IMMEDIATE || item->form == V30_ALU_GROUP_SIGNED ||
            item->form == V30_ALU_UNARY_GROUP) form |= ((lib_u32)(item->modrm >> 3u) & 7u) << 9u;
    }
    const x86_core_cpu_timing_result timing = x86_core_cpu_capture_timing(&fixture->bus.execution);
    if (timing.ticks != clocks || !timing.source_timing_unallocated || timing.event_ticks ||
        timing.retirement_origin != (clocks ? X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE :
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED) ||
        timing.form_id != form || timing.key_id != form ||
        timing.formula_inputs != (clocks && memory ? X86_CORE_CPU_TIMING_INPUT_MODRM |
            X86_CORE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS | (odd ? X86_CORE_CPU_TIMING_INPUT_ODD_WORD : 0u) : 0u) ||
        x86_core_cpu_timing_maximum_ticks(X86_CORE_CPU_PROFILE_V30,LIB_NULL)) {
        lib_c_printf("V30:NOMINAL-ALU:FAIL op=%u form=%u opcode=%02x modrm=%02x prefix=%u ticks=%llu/%llu key=%x/%x\n",
            item->operation,item->form,item->code[0],item->modrm,item->prefix,timing.ticks,clocks,timing.form_id,form);
        return 1;
    }
    return 0;
}

static lib_i32 v30_alu_check(const v30_alu_case *item, lib_bool deep)
{
    static v30_memory_fixture fixture;
    v30_alu_prepare(&fixture,item);
    const t_cpu before = fixture.bus.cpu;
    t_cpu expected = before;
    const lib_bool memory = item->form != V30_ALU_ACC_IMMEDIATE && item->form < V30_ALU_EXTEND && (item->modrm >> 6u) != 3u;
    const lib_u32 address = memory ? v30_alu_address(&expected,item) : 0u;
    const lib_u16 initial = memory ? fixture.memory[address] + 256u*fixture.memory[address+1u] : 0u;
    const lib_u8 rm = item->modrm & 7u, reg = (item->modrm >> 3u) & 7u;
    const lib_u16 operand = memory ? (item->width == 1u ? initial & 255u : initial) :
        v30_alu_get(&expected,rm,item->width);
    const lib_u16 left = item->form == V30_ALU_REGISTER_RM ? v30_alu_get(&expected,reg,item->width) :
        item->form == V30_ALU_ACC_IMMEDIATE || item->form == V30_ALU_EXTEND ? v30_alu_get(&expected,0u,item->width) : operand;
    const lib_u16 right = item->form < V30_ALU_ACC_IMMEDIATE ? (item->form == V30_ALU_REGISTER_RM ? operand :
        v30_alu_get(&expected,reg,item->width)) : item->immediate;
    const lib_bool store = item->operation != V30_ALU_CMP && item->operation != V30_ALU_TEST;
    const lib_bool write = memory && item->form != V30_ALU_REGISTER_RM && store;
    lib_u32 defined;
    const lib_u16 value = v30_alu_oracle(item->operation,item->width,left,right,
        &expected.data.eflags,&defined);
    if (store && !write) v30_alu_set(&expected,item->form == V30_ALU_REGISTER_RM ? reg :
        item->form == V30_ALU_ACC_IMMEDIATE || item->operation == V30_ALU_CBW ? 0u :
        item->operation == V30_ALU_CWD ? 2u : rm,
        item->operation >= V30_ALU_CBW ? 2u : item->width,value);
    expected.data.eip += item->length;
    if (deep) {
        x86_core_cpu_instruction_lexeme lexeme;
        if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
            lexeme.byte_count != item->length || fixture.fetches || fixture.reads || fixture.writes || fixture.phase_count ||
            lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) return 1;
        for (lib_u32 index = 0u; index < fixture.record_count; ++index)
            if (!fixture.records[index].observe ||
                fixture.records[index].provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) return 1;
        fixture.record_count = fixture.observations = 0u;
    }
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    t_cpu actual = fixture.bus.cpu;
    actual.data.eflags &= defined;
    expected.data.eflags &= defined;
    if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.bus.delivered_exceptions ||
        fixture.bus.instruction_count != 1u ||
        fixture.bus.execution.interrupt_shadow != CPU_INTERRUPT_SHADOW_NONE ||
        v30_alu_timing(&fixture,item,memory,address) ||
        lib_memory_compare(&actual,&expected,sizeof(actual)) ||
        v30_alu_records(&fixture,address,item->width,memory,write,initial,value,LIB_FALSE)) {
        lib_c_printf("V30:ALU:FAIL op=%u form=%u width=%u modrm=%02x left=%04x imm=%04x flags=%08x/%08x result=%08x/%08x\n",
            item->operation,item->form,item->width,item->modrm,item->left,item->immediate,
            actual.data.eflags,expected.data.eflags,actual.data.eax,expected.data.eax);
        return 1;
    }
    const lib_u32 counts[] = {fixture.reads,fixture.writes,fixture.fetches};
    lib_u32 reads_before_fetch[sizeof(item->code)] = {0}, fetched = 0u, read_prefix = 0u;
    if (deep) for (lib_u32 index = 0u; index < fixture.record_count; ++index) {
        const v30_memory_record *record = &fixture.records[index];
        if (record->observe) continue;
        if (record->provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) {
            if (!record->write) ++read_prefix;
        } else {
            if (fetched >= sizeof(reads_before_fetch)/sizeof(reads_before_fetch[0])) return 1;
            reads_before_fetch[fetched++] = read_prefix;
        }
    }
    if (deep) for (lib_u8 kind = 0u; kind < 3u; ++kind)
        for (lib_u32 part = 1u; part <= counts[kind]; ++part) {
            v30_alu_prepare(&fixture,item);
            if (kind == 0u) fixture.fail_read = part;
            else if (kind == 1u) fixture.fail_write = part;
            else fixture.fail_fetch = part;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || !fixture.bus.faults || fixture.bus.delivered_exceptions ||
                fixture.bus.execution.interrupt_shadow != CPU_INTERRUPT_SHADOW_NONE ||
                lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before)) ||
                fixture.bus.execution.completed_observation_valid ||
                fixture.bus.execution.timing_result.ticks ||
                fixture.bus.execution.timing_result.retirement_origin != X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
                (kind == 2u ? fixture.reads != reads_before_fetch[part-1u] || fixture.writes ||
                    v30_alu_records(&fixture,address,item->width,reads_before_fetch[part-1u] != 0u,
                        LIB_FALSE,initial,initial,LIB_FALSE) :
                    v30_alu_records(&fixture,address,item->width,memory,write,initial,value,LIB_TRUE))) {
                lib_c_printf("V30:NOMINAL-ALU:REFUSAL op=%u form=%u modrm=%02x prefix=%u kind=%u part=%u stop=%u fault=%u pc=%x/%x reads=%u writes=%u complete=%u ticks=%llu\n",
                    item->operation,item->form,item->modrm,item->prefix,kind,part,
                    fixture.bus.execution.stop_requested,fixture.bus.faults,fixture.bus.cpu.data.eip,before.data.eip,
                    fixture.reads,fixture.writes,fixture.bus.execution.completed_observation_valid,
                    fixture.bus.execution.timing_result.ticks);
                return 1;
            }
        }
    return 0;
}

static v30_alu_case v30_alu_build(v30_alu_operation operation, lib_u8 width,
    v30_alu_form form, lib_u8 modrm, lib_u8 prefix, lib_u16 value, lib_bool carry)
{
    static const lib_u8 prefixes[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
    const lib_u8 mod = modrm >> 6u, rm = modrm & 7u;
    v30_alu_case item = {.operation=operation,.width=width,.form=form,.modrm=modrm,
        .prefix=prefix,.immediate=value,.left=value,.carry=carry};
    if (prefix) item.code[item.length++] = prefixes[prefix];
    lib_u8 opcode = operation < V30_ALU_TEST ? 8u*operation + (width-1u) +
        (form == V30_ALU_REGISTER_RM ? 2u : form == V30_ALU_ACC_IMMEDIATE ? 4u : 0u) : 0x84u+width-1u;
    if (form == V30_ALU_GROUP_IMMEDIATE || form == V30_ALU_GROUP_SIGNED) opcode = operation == V30_ALU_TEST ? 0xf5u+width :
        form == V30_ALU_GROUP_SIGNED ? 0x83u : 0x7fu+width;
    if (form == V30_ALU_ACC_IMMEDIATE && operation == V30_ALU_TEST) opcode = 0xa7u+width;
    if (form == V30_ALU_UNARY_GROUP) opcode = operation == V30_ALU_INC || operation == V30_ALU_DEC ?
        0xfdu+width : 0xf5u+width;
    if (form == V30_ALU_UNARY_REGISTER) opcode = (operation == V30_ALU_INC ? 0x40u : 0x48u) + rm;
    if (form == V30_ALU_EXTEND) opcode = operation == V30_ALU_CBW ? 0x98u : 0x99u;
    item.code[item.length++] = opcode;
    if (form != V30_ALU_ACC_IMMEDIATE && form < V30_ALU_UNARY_REGISTER) {
        item.code[item.length++] = modrm;
        if (mod < 3u && (mod || rm == 6u)) {
            item.code[item.length++] = mod == 1u ? 0x80u : 0xffu;
            if (mod != 1u) item.code[item.length++] = 0xffu;
        }
    }
    if (form == V30_ALU_ACC_IMMEDIATE || form == V30_ALU_GROUP_IMMEDIATE || form == V30_ALU_GROUP_SIGNED) {
        item.code[item.length++] = (lib_u8)value;
        if (width == 2u && form != V30_ALU_GROUP_SIGNED) item.code[item.length++] = (lib_u8)(value >> 8u);
        if (form == V30_ALU_GROUP_SIGNED) item.immediate = (lib_u16)(lib_i16)(lib_i8)value;
    }
    return item;
}

lib_i32 main(void)
{
    /* Adjacent dynamic/alias/prefix forms must not inherit fixed scalar rows. */
    static const lib_u8 unallocated[][4] = {
        {0x82u,0xc0u,0x01u}, {0xf6u,0xc8u,0x01u}, {0xf7u,0xc8u,0x01u,0x00u},
        {0xf6u,0xe0u}, {0xf7u,0xe0u}, {0xf6u,0xf3u}, {0xf7u,0xfbu},
        {0xd0u,0xf0u}, {0xc1u,0xf0u,0x04u}, {0x0fu,0x10u,0xc0u},
        {0xffu,0xd8u}, {0xf0u,0x01u,0x07u}, {0xf3u,0x04u,0x01u}, {0x2eu,0x98u}, {0x99u},
        {0xd4u,9u}, {0xd5u,9u}, {0x27u}, {0x2fu}
    };
    static v30_memory_fixture boundary;
    v30_memory_prepare(&boundary);
    for (lib_size index = 0u; index < sizeof(unallocated)/sizeof(unallocated[0]); ++index) {
        v30_memory_restart(&boundary);
        boundary.bus.cpu.data.ax = 5u;
        boundary.bus.cpu.data.bx = 3u;
        lib_memory_copy(boundary.memory+0x80100u,unallocated[index],sizeof(unallocated[index]));
        x86_core_cpu_execution_refresh(&boundary.bus.execution);
        const x86_core_cpu_timing_result timing = x86_core_cpu_capture_timing(&boundary.bus.execution);
        /* F6/F7 /1, group-2 /6 and far register CALL remain prohibited. */
        const lib_bool rejected = index == 1u || index == 2u || index == 7u || index == 8u || index == 10u;
        if (boundary.bus.execution.stop_requested != rejected || boundary.bus.faults != (lib_u32)rejected ||
            (rejected && boundary.bus.execution.completed_observation_valid) || timing.ticks ||
            (!rejected && (!timing.source_timing_unallocated ||
                timing.key_id != X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED)) ||
            timing.form_id != timing.key_id || timing.formula_inputs ||
            timing.retirement_origin != X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED) {
            lib_c_printf("V30:NOMINAL-ALU:EXCLUDED index=%u stop=%u ticks=%llu\n",
                (lib_u32)index,boundary.bus.execution.stop_requested,timing.ticks);
            return 1;
        }
    }
    static const lib_u16 values[] = {0u,1u,15u,16u,127u,128u,255u,256u,
        0x7fffu,0x8000u,0xfffeu,0xffffu};
    lib_u32 cases = 0u;
    for (lib_u8 operation = 0u; operation <= V30_ALU_TEST; ++operation)
        for (lib_u8 width = 1u; width <= 2u; ++width)
            for (lib_u8 form = 0u; form <= V30_ALU_GROUP_SIGNED; ++form) {
                if (form == V30_ALU_GROUP_SIGNED && (width == 1u || operation == V30_ALU_TEST)) continue;
                if (form == V30_ALU_REGISTER_RM && operation == V30_ALU_TEST) continue;
                for (lib_u16 modrm = 0u; modrm <= 255u; ++modrm) {
                    if (form == V30_ALU_ACC_IMMEDIATE && modrm) continue;
                    if (form >= V30_ALU_GROUP_IMMEDIATE && ((modrm >> 3u) & 7u) !=
                        (operation == V30_ALU_TEST ? 0u : operation)) continue;
                    for (lib_u8 prefix = 0u; prefix < 5u; ++prefix)
                        for (lib_u8 carry = 0u; carry < 2u; ++carry) {
                            const v30_alu_case item = v30_alu_build((v30_alu_operation)operation,
                                width,form,(lib_u8)modrm,prefix,carry ? 0xffffu : 0x8001u,carry != 0u);
                            if (v30_alu_check(&item,LIB_TRUE)) return 1;
                            ++cases;
                        }
                }
            }
    /* Every byte pair; ADC/SBB additionally use both input carries. */
    for (lib_u8 operation = 0u; operation <= V30_ALU_TEST; ++operation)
        for (lib_u16 left = 0u; left <= 255u; ++left)
            for (lib_u16 right = 0u; right <= 255u; ++right)
                for (lib_u8 carry = 0u; carry <
                    (operation == V30_ALU_ADC || operation == V30_ALU_SBB ? 2u : 1u); ++carry) {
                    v30_alu_case item = v30_alu_build((v30_alu_operation)operation,1u,V30_ALU_ACC_IMMEDIATE,0u,0u,right,carry != 0u);
                    item.left = left;
                    item.seed_accumulator = LIB_TRUE;
                    if (v30_alu_check(&item,LIB_FALSE)) return 1;
                    ++cases;
                }
    for (lib_u8 operation = 0u; operation <= V30_ALU_TEST; ++operation)
        for (lib_u8 left = 0u; left < sizeof(values)/sizeof(values[0]); ++left)
            for (lib_u8 right = 0u; right < sizeof(values)/sizeof(values[0]); ++right)
                for (lib_u8 carry = 0u; carry < 2u; ++carry) {
                    v30_alu_case item = v30_alu_build((v30_alu_operation)operation,2u,V30_ALU_ACC_IMMEDIATE,0u,0u,values[right],carry != 0u);
                    item.left = values[left];
                    item.seed_accumulator = LIB_TRUE;
                    if (v30_alu_check(&item,LIB_TRUE)) return 1;
                    ++cases;
                }
    for (lib_u8 operation = V30_ALU_INC; operation <= V30_ALU_NOT; ++operation)
        for (lib_u8 width = 1u; width <= 2u; ++width)
            for (lib_u8 mod = 0u; mod < 4u; ++mod)
                for (lib_u8 rm = 0u; rm < 8u; ++rm)
                    for (lib_u8 prefix = 0u; prefix < 5u; ++prefix)
                        for (lib_u8 value = 0u; value < sizeof(values)/sizeof(values[0]); ++value)
                            for (lib_u8 carry = 0u; carry < 2u; ++carry) {
                                const lib_u8 extension = operation == V30_ALU_INC ? 0u :
                                    operation == V30_ALU_DEC ? 1u : operation == V30_ALU_NOT ? 2u : 3u;
                                v30_alu_case item = v30_alu_build((v30_alu_operation)operation,
                                    width,V30_ALU_UNARY_GROUP,(mod << 6u) | (extension << 3u) | rm,prefix,values[value],carry != 0u);
                                if (v30_alu_check(&item,LIB_TRUE)) return 1;
                                ++cases;
                                if (mod == 3u && width == 2u && operation <= V30_ALU_DEC) {
                                    item = v30_alu_build((v30_alu_operation)operation,width,V30_ALU_UNARY_REGISTER,
                                        (lib_u8)(0xc0u|rm),prefix,values[value],carry != 0u);
                                    if (v30_alu_check(&item,LIB_TRUE)) return 1;
                                    ++cases;
                                }
                            }
    for (lib_u8 operation = V30_ALU_CBW; operation <= V30_ALU_CWD; ++operation)
        for (lib_u32 value = 0u; value < (operation == V30_ALU_CBW ? 256u : 65536u); ++value) {
            v30_alu_case item = v30_alu_build((v30_alu_operation)operation,
                operation == V30_ALU_CBW ? 1u : 2u,V30_ALU_EXTEND,0u,0u,0u,value & 1u);
            item.left = (lib_u16)value;
            item.seed_accumulator = LIB_TRUE;
            if (v30_alu_check(&item,LIB_TRUE)) return 1;
            ++cases;
        }
    lib_c_printf("V30:ALU:OK cases=%u\n",cases);
    lib_c_printf("V30:NOMINAL-ALU:OK cases=%u\n",cases);
    lib_c_printf("V30:NOMINAL-ALU:BOUNDARIES:OK cases=%u\n",(lib_u32)(sizeof(unallocated)/sizeof(unallocated[0])));
    return 0;
}
