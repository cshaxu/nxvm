#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

typedef enum v30_entry_operation {
    V30_ENTRY_BREAKPOINT, V30_ENTRY_SOFTWARE, V30_ENTRY_OVERFLOW,
    V30_ENTRY_BOUNDS, V30_ENTRY_RETURN
} v30_entry_operation;

typedef struct v30_entry_case {
    v30_entry_operation operation;
    lib_u16 pc, sp, index, lower, upper, flags, target, segment;
    lib_u8 vector, modrm, prefix;
} v30_entry_case;

static lib_u16 *v30_entry_register(t_cpu *cpu, lib_u8 reg)
{
    lib_u16 *registers[] = {&cpu->data.ax,&cpu->data.cx,&cpu->data.dx,&cpu->data.bx,
        &cpu->data.sp,&cpu->data.bp,&cpu->data.si,&cpu->data.di};
    return registers[reg];
}

static lib_u32 v30_entry_address(const t_cpu *cpu, const v30_entry_case *item)
{
    const lib_u8 mod = item->modrm >> 6u, rm = item->modrm & 7u;
    const lib_u16 offsets[] = {cpu->data.bx+cpu->data.si,cpu->data.bx+cpu->data.di,
        cpu->data.bp+cpu->data.si,cpu->data.bp+cpu->data.di,cpu->data.si,
        cpu->data.di,cpu->data.bp,cpu->data.bx};
    const lib_bool stack = rm == 2u || rm == 3u || (rm == 6u && mod != 0u);
    const lib_u32 bases[] = {stack ? 0x20000u : 0x10000u,0x30000u,0x80000u,0x20000u,0x10000u};
    return bases[item->prefix]+(lib_u16)(mod == 0u && rm == 6u ? 0x501u :
        offsets[rm]+(mod == 1u ? 1u : mod == 2u ? 0xffffu : 0u));
}

static lib_u16 v30_entry_flags(lib_u16 bits)
{
    static const lib_u16 masks[] = {VCPU_EFLAGS_CF,VCPU_EFLAGS_PF,VCPU_EFLAGS_AF,
        VCPU_EFLAGS_ZF,VCPU_EFLAGS_SF,VCPU_EFLAGS_IF,VCPU_EFLAGS_DF,VCPU_EFLAGS_OF};
    lib_u16 flags = 0u;
    for (lib_u8 bit = 0u; bit < sizeof(masks)/sizeof(masks[0]); ++bit)
        if (bits & (1u << bit)) flags |= masks[bit];
    return flags;
}

static void v30_entry_store(v30_memory_fixture *fixture, lib_u32 address, lib_u16 value)
{
    fixture->memory[address] = (lib_u8)value;
    fixture->memory[address+1u] = (lib_u8)(value >> 8u);
}

static lib_u16 v30_entry_word(const v30_memory_fixture *fixture, lib_u32 address)
{
    return fixture->memory[address]+256u*fixture->memory[address+1u];
}

static lib_u8 v30_entry_prepare(v30_memory_fixture *fixture, const v30_entry_case *item)
{
    static const lib_u8 opcodes[] = {0xccu,0xcdu,0xceu,0x62u,0xcfu};
    static const lib_u8 prefixes[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
    v30_memory_restart(fixture);
    fixture->bus.cpu.data.eax = 0x12340123u;
    fixture->bus.cpu.data.ecx = 0x23451234u;
    fixture->bus.cpu.data.edx = 0x34562345u;
    fixture->bus.cpu.data.ebx = 0x45673456u;
    fixture->bus.cpu.data.ebp = 0x56784567u;
    fixture->bus.cpu.data.esi = 0x67895678u;
    fixture->bus.cpu.data.edi = 0x789a6789u;
    fixture->bus.cpu.data.esp = 0x89ab0000u | item->sp;
    fixture->bus.cpu.data.eip = item->pc;
    fixture->bus.cpu.data.eflags = 0x8002u | item->flags;
    if (item->operation == V30_ENTRY_BOUNDS)
        *v30_entry_register(&fixture->bus.cpu,(item->modrm >> 3u) & 7u) = item->index;
    for (lib_i8 word = -3; word <= 3; ++word)
        v30_entry_store(fixture,0x20000u+(lib_u16)(fixture->bus.cpu.data.sp+2*word),0xa55au);
    if (item->operation == V30_ENTRY_RETURN) {
        const lib_u16 words[] = {item->target,item->segment,item->index};
        for (lib_u8 word = 0u; word < 3u; ++word)
            v30_entry_store(fixture,0x20000u+(lib_u16)(item->sp+2u*word),words[word]);
    }
    if (item->operation == V30_ENTRY_BOUNDS) {
        const lib_u32 address = v30_entry_address(&fixture->bus.cpu,item);
        v30_entry_store(fixture,address,item->lower);
        v30_entry_store(fixture,address+2u,item->upper);
    }
    const lib_u8 vector = item->operation == V30_ENTRY_BREAKPOINT ? 3u :
        item->operation == V30_ENTRY_OVERFLOW ? 4u : item->operation == V30_ENTRY_BOUNDS ? 5u : item->vector;
    v30_entry_store(fixture,4u*vector,0x200u);
    v30_entry_store(fixture,4u*vector+2u,0x9000u);
    fixture->memory[0x90200u] = 0xcfu;
    fixture->memory[0x90201u] = 0x90u;
    lib_u8 code[6], length = 0u;
    if (item->prefix) code[length++] = prefixes[item->prefix];
    code[length++] = opcodes[item->operation];
    if (item->operation == V30_ENTRY_SOFTWARE) code[length++] = item->vector;
    if (item->operation == V30_ENTRY_BOUNDS) {
        code[length++] = item->modrm;
        const lib_u8 mod = item->modrm >> 6u;
        if (mod || (item->modrm & 7u) == 6u) {
            code[length++] = mod == 1u ? 1u : mod == 0u ? 1u : 0xffu;
            if (mod != 1u) code[length++] = mod == 0u ? 5u : 0xffu;
        }
    }
    for (lib_u8 byte = 0u; byte < 8u; ++byte)
        fixture->memory[0x80000u+(lib_u16)(item->pc+byte)] = 0x90u;
    for (lib_u8 byte = 0u; byte < length; ++byte)
        fixture->memory[0x80000u+(lib_u16)(item->pc+byte)] = code[byte];
    return length;
}

static lib_i32 v30_entry_timing(const v30_memory_fixture *fixture, const v30_entry_case *item,
    const t_cpu *before, lib_bool taken)
{
    static const lib_u8 opcodes[] = {0xccu,0xcdu,0xceu,0x62u,0xcfu};
    lib_u64 clocks = 0u;
    lib_u32 key = X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED, inputs = 0u;
    x86_core_cpu_retirement_timing_origin origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED;
    const lib_bool stack_odd = (before->data.sp & 1u) != 0u;
    key = opcodes[item->operation];
    origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE;
    inputs = X86_CORE_CPU_TIMING_INPUT_CONTROL | X86_CORE_CPU_TIMING_INPUT_BRANCH;
    if (taken || item->operation == V30_ENTRY_RETURN) inputs |= X86_CORE_CPU_TIMING_INPUT_BRANCH_TAKEN;
    if (item->operation == V30_ENTRY_BOUNDS) {
        const lib_bool operand_odd = (v30_entry_address(before,item) & 1u) != 0u;
        clocks = taken ? (operand_odd || stack_odd ? 76u : 56u) : operand_odd ? 26u : 18u;
        key |= 0x100u;
        inputs |= X86_CORE_CPU_TIMING_INPUT_MODRM | X86_CORE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS;
        if (operand_odd || (taken && stack_odd)) inputs |= X86_CORE_CPU_TIMING_INPUT_ODD_WORD;
        origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL;
    } else if (item->operation == V30_ENTRY_OVERFLOW && !taken) {
        clocks = 3u;
        origin = X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL;
    } else {
        clocks = (item->operation == V30_ENTRY_RETURN ? 27u : item->operation == V30_ENTRY_OVERFLOW ? 40u : 38u) +
            (stack_odd ? 12u : 0u);
        if (stack_odd) inputs |= X86_CORE_CPU_TIMING_INPUT_ODD_WORD;
    }
    if (item->operation != V30_ENTRY_OVERFLOW || taken) inputs |= X86_CORE_CPU_TIMING_INPUT_MEMORY;
    if (item->prefix) { clocks+=2u; inputs|=X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE; }
    return v30_memory_check_nominal(fixture,clocks,key,inputs,origin);
}

static lib_i32 v30_entry_check(const v30_entry_case *item, lib_bool deep)
{
    static v30_memory_fixture fixture;
    const lib_u8 length = v30_entry_prepare(&fixture,item);
    const t_cpu before = fixture.bus.cpu;
    t_cpu expected = before;
    v30_memory_plan plan = {0};
    lib_bool taken = item->operation < V30_ENTRY_RETURN;
    if (item->operation == V30_ENTRY_BOUNDS) {
        const lib_u32 address = v30_entry_address(&expected,item);
        const lib_u16 lower = v30_entry_word(&fixture,address), upper = v30_entry_word(&fixture,address+2u);
        /* Retain the existing signed comparator model; entry is this batch's owner. */
        taken = (lib_i16)item->index < (lib_i16)lower || (lib_i16)item->index > (lib_i16)upper;
        v30_memory_plan_transfer(&plan,&fixture,address,2u,LIB_FALSE,lower);
        v30_memory_plan_transfer(&plan,&fixture,address+2u,2u,LIB_FALSE,upper);
    }
    if (item->operation == V30_ENTRY_OVERFLOW) taken = (item->flags & VCPU_EFLAGS_OF) != 0u;
    const lib_u16 next = (lib_u16)(item->pc+length);
    if (taken) {
        const lib_u16 words[] = {(lib_u16)(before.data.eflags | 0x7000u),0x8000u,next};
        for (lib_u8 word = 0u; word < 3u; ++word) {
            expected.data.sp -= 2u;
            v30_memory_plan_transfer(&plan,&fixture,0x20000u+expected.data.sp,2u,LIB_TRUE,words[word]);
        }
        const lib_u8 vector = item->operation == V30_ENTRY_BREAKPOINT ? 3u :
            item->operation == V30_ENTRY_OVERFLOW ? 4u : item->operation == V30_ENTRY_BOUNDS ? 5u : item->vector;
        v30_memory_plan_transfer(&plan,&fixture,4u*vector,2u,LIB_FALSE,0x200u);
        v30_memory_plan_transfer(&plan,&fixture,4u*vector+2u,2u,LIB_FALSE,0x9000u);
        expected.data.cs.selector = 0x9000u;
        expected.data.cs.base = 0x90000u;
        expected.data.eip = 0x200u;
        expected.data.eflags &= ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_TF);
    } else if (item->operation == V30_ENTRY_RETURN) {
        const lib_u16 words[] = {item->target,item->segment,item->index};
        for (lib_u8 word = 0u; word < 3u; ++word)
            v30_memory_plan_transfer(&plan,&fixture,0x20000u+(lib_u16)(before.data.sp+2u*word),
                2u,LIB_FALSE,words[word]);
        expected.data.sp += 6u;
        expected.data.cs.selector = item->segment;
        expected.data.cs.base = 16u*item->segment;
        expected.data.eip = item->target;
        expected.data.eflags = 0x8002u | (item->index & 0x0fd5u);
    } else expected.data.eip = next;
    lib_u32 observations = 0u;
    if (deep) {
        x86_core_cpu_instruction_lexeme lexeme;
        if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
            lexeme.byte_count != length || fixture.reads || fixture.writes || fixture.fetches || fixture.phase_count ||
            fixture.bus.instruction_count || fixture.bus.faults || fixture.bus.delivered_exceptions ||
            lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) return 1;
        for (lib_u32 index = 0u; index < fixture.record_count; ++index)
            if (!fixture.records[index].observe ||
                fixture.records[index].provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) return 1;
        observations = fixture.observations;
        fixture.record_count = fixture.observations = 0u;
    }
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (v30_entry_timing(&fixture,item,&before,taken)) return 1;
    const lib_bool diagnostic = taken && item->operation == V30_ENTRY_BOUNDS;
    if (fixture.bus.execution.stop_requested || fixture.bus.faults ||
        fixture.bus.delivered_exceptions != (lib_u32)diagnostic ||
        (diagnostic && fixture.bus.delivered_exception.exception_mask != VCPUINS_EXCEPT_BR) ||
        fixture.bus.instruction_count != 1u || v30_memory_check_completion(&fixture,LIB_TRUE,LIB_FALSE) ||
        fixture.bus.execution.interrupt_shadow != CPU_INTERRUPT_SHADOW_NONE ||
        lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) || v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) {
        const lib_u32 frame = 0x20000u+fixture.bus.cpu.data.sp;
        lib_c_printf("V30:SOFTWARE-ENTRY:FAIL op=%u vector=%u modrm=%02x prefix=%u pc=%04x sp=%04x ip=%04x/%04x flags=%04x/%04x saved=%04x expected-next=%04x plan=%u\n",
            item->operation,item->vector,item->modrm,item->prefix,item->pc,before.data.sp,
            fixture.bus.cpu.data.eip,expected.data.eip,fixture.bus.cpu.data.eflags,expected.data.eflags,
            v30_entry_word(&fixture,frame),next,v30_memory_check_plan(&fixture,&plan,LIB_FALSE));
        return 1;
    }
    const lib_u32 reads = fixture.reads, writes = fixture.writes, fetches = fixture.fetches;
    if (deep && taken) {
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        t_cpu returned = before;
        returned.data.eip = next;
        if (fixture.bus.execution.stop_requested || lib_memory_compare(&fixture.bus.cpu,&returned,sizeof(returned))) return 1;
    }
    if (deep) for (lib_u8 kind = 0u; kind < 4u; ++kind) {
        const lib_u32 counts[] = {reads,writes,fetches,observations};
        for (lib_u32 part = 1u; part <= counts[kind]; ++part) {
            v30_entry_prepare(&fixture,item);
            if (kind == 0u) fixture.fail_read = part;
            if (kind == 1u) fixture.fail_write = part;
            if (kind == 2u) fixture.fail_fetch = part;
            if (kind == 3u) {
                x86_core_cpu_instruction_lexeme lexeme;
                fixture.fail_observe = part;
                if (x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
                    fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.fetches) return 1;
            } else {
                x86_core_cpu_execution_refresh(&fixture.bus.execution);
                if (!fixture.bus.execution.stop_requested || !fixture.bus.faults ||
                    fixture.bus.execution.timing_result.ticks ||
                    fixture.bus.execution.timing_result.retirement_origin != X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
                    v30_memory_check_completion(&fixture,LIB_FALSE,LIB_FALSE)) return 1;
            }
            if (fixture.bus.delivered_exceptions || fixture.bus.execution.interrupt_shadow != CPU_INTERRUPT_SHADOW_NONE ||
                lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before)) ||
                (kind < 2u ? v30_memory_check_plan(&fixture,&plan,LIB_TRUE) :
                    fixture.reads || fixture.writes || fixture.phase_count)) {
                lib_c_printf("V30:SOFTWARE-ENTRY:REFUSAL op=%u vector=%u modrm=%02x prefix=%u pc=%04x sp=%04x kind=%u part=%u\n",
                    item->operation,item->vector,item->modrm,item->prefix,item->pc,before.data.sp,kind,part);
                return 1;
            }
        }
    }
    return 0;
}

lib_i32 main(void)
{
    static const lib_u16 pcs[] = {0x100u,1u,0xfffcu,0xffffu};
    static const lib_u16 stacks[] = {0x700u,0x701u,0u,1u,0xffffu};
    static const lib_u16 values[] = {0u,1u,37u,0x8000u,0xffffu};
    static const lib_u16 segments[] = {0u,0x8000u,0x9000u,0xffffu};
    static const lib_u16 bounds[][2] = {{0u,0u},{0u,0x7fffu},{5u,37u},{100u,99u}};
    lib_u32 cases = 0u;
    const v30_entry_case witness = {.operation=V30_ENTRY_BOUNDS,.pc=0x100u,.sp=0x701u,
        .index=87u,.lower=2u,.upper=80u,.modrm=0x06u,.flags=VCPU_EFLAGS_IF};
    if (v30_entry_check(&witness,LIB_TRUE)) return 1;
    ++cases;
    for (lib_u8 operation = 0u; operation <= V30_ENTRY_RETURN; ++operation)
        for (lib_u16 flags = 0u; flags < (operation == V30_ENTRY_RETURN ? 2u :
            operation == V30_ENTRY_BOUNDS ? 1u : 256u); ++flags)
            for (lib_u32 value = 0u; value < (operation == V30_ENTRY_BOUNDS || operation == V30_ENTRY_RETURN ? 65536u :
                operation == V30_ENTRY_SOFTWARE ? 256u : 1u); ++value)
                for (lib_u8 bound = 0u; bound < (operation == V30_ENTRY_BOUNDS ? 4u : 1u); ++bound) {
                    const v30_entry_case item = {.operation=(v30_entry_operation)operation,.pc=0x100u,.sp=0x701u,
                        .index=(lib_u16)value,.lower=bounds[bound][0],.upper=bounds[bound][1],.modrm=0x06u,
                        .flags=v30_entry_flags(operation == V30_ENTRY_RETURN ? 255u*flags : flags),
                        .vector=(lib_u8)value,.target=0x3456u,.segment=0x9000u};
                    if (v30_entry_check(&item,LIB_FALSE)) return 1;
                    ++cases;
                }
    for (lib_u8 operation = 0u; operation <= V30_ENTRY_RETURN; ++operation)
        for (lib_u8 prefix = 0u; prefix < 5u; ++prefix)
            for (lib_u8 pc = 0u; pc < sizeof(pcs)/sizeof(pcs[0]); ++pc)
                for (lib_u8 stack = 0u; stack < sizeof(stacks)/sizeof(stacks[0]); ++stack)
                    for (lib_u8 value = 0u; value < sizeof(values)/sizeof(values[0]); ++value)
                        for (lib_u8 flags = 0u; flags < 2u; ++flags)
                            for (lib_u16 modrm = 0u; modrm < (operation == V30_ENTRY_BOUNDS ? 192u : 1u); ++modrm) {
                                const v30_entry_case item = {.operation=(v30_entry_operation)operation,.pc=pcs[pc],
                                    .sp=stacks[stack],.index=values[value],.lower=5u,.upper=37u,.modrm=(lib_u8)modrm,
                                    .prefix=prefix,.flags=v30_entry_flags(255u*flags),.vector=(lib_u8)values[value],
                                    .target=values[value],.segment=segments[value % 4u]};
                                if (v30_entry_check(&item,LIB_TRUE)) return 1;
                                ++cases;
                            }
    lib_c_printf("V30:SOFTWARE-ENTRY:OK cases=%u\n",cases);
    lib_c_printf("V30:NOMINAL-SOFTWARE-CONTROL:OK\n");
    return 0;
}
